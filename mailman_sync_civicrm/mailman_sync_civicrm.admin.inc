<?php

function mailman_sync_civicrm_relationship_form($form_state) {
  if (!civicrm_initialize()) {
    drupal_set_message(t("CiviCRM could not be initialized."), 'error');
  }
  $relationship_types = _mailman_sync_civicrm_get_relationship_types();

  $form = array();
  $form['fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Create new relationship pairing'),
    'relationship_type' => array(
      '#type' => 'select',
      '#title' => 'Relationship type',
      '#required' => TRUE,
      '#options' => $relationship_types,
    ),
    'contact_b' => array(
      '#type' => 'textfield',
      '#title' => 'Contact b',
      '#autocomplete_path' => 'admin/config/mailman/cvicicrm-relationships/autocomplete',
      '#description' => t('Optional. If blank, this will include all contacts with specified relationship type.'),
    ),
    'save' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
    ),
  );

  $results = db_query("SELECT id, relationship_type_id, contact_b FROM {mailman_sync_civicrm}")->fetchAll();
  $header = array('Id', 'Civicrm Relationship', 'Contact B', 'Delete');
  $rows = array();

  if (!empty($results)) {
    foreach ($results as $result) {
      $contact_name = '';
      if ($contact = _mailman_sync_civicrm_get_contact($result->contact_b)) {
        $contact_name = $contact['display_name'];
      }

      $rows[] = array(
        $result->id,
        "{$relationship_types[$result->relationship_type_id]} [id:{$result->relationship_type_id}]",
        "$contact_name" . (empty($result->contact_b) ? '*' : " [id:{$result->contact_b}]"),
        l('Delete', "admin/config/mailman/civicrm-relationships/delete/{$result->id}"),
      );
    }
  }
  else {
    $rows[] = array(
      array('data' => "No relationships yet exist.", 'colspan' => 4)
    );
  }
  $form['relationships'] = array(
    '#type' => 'markup',
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  return $form;
}

function mailman_sync_civicrm_relationship_form_validate($form, &$form_state) {
  // Try to extract contact_b id unless contact_b is empty
  $contact_b = $form_state['values']['contact_b'];
  if (!empty($contact_b)) {
    $matches = array();
    preg_match('/.*\[id:(\d+)\]/', $contact_b, $matches);

    if (empty($matches[1])) {
      form_set_error('contact_b', "Contact b must be in the form of 'John Smith [id:1234]'");
    }
    else {
      form_set_value($form['fieldset']['contact_b'], $matches[1], $form_state);
    }
  }
}

function mailman_sync_civicrm_relationship_form_submit($form, &$form_state) {
  $record = array();
  $record['relationship_type_id'] = $form_state['values']['relationship_type'];
  $record['contact_b'] = $form_state['values']['contact_b'] ? $form_state['values']['contact_b'] : NULL;

  try {
    drupal_write_record('mailman_sync_civicrm', $record);
    cache_clear_all('mailman_sync_groups', 'cache');
  }
  catch (PDOException $e) {
    // Do nothing.
  }
}

function _mailman_sync_civicrm_autocomplete($string) {
  if (!civicrm_initialize()) {
    drupal_exit();
  }

  $result = civicrm_api('contact', 'get', array(
    'version' => 3,
    'display_name' => "%{$string}%",
  ));

  if (!empty($result['is_error'])) {
    drupal_exit();
  }

  $contacts = array();
  foreach ($result['values'] as $value) {
    $name = "{$value['display_name']} [id:{$value['id']}]";
    $contacts[$name] = check_plain($name);
  }

  drupal_json_output($contacts);
}

function _mailman_sync_civicrm_relationship_delete($id) {
  db_delete('mailman_sync_civicrm')
    ->condition('id', $id)
    ->execute();
  cache_clear_all('mailman_sync_groups', 'cache');

  drupal_goto('admin/config/mailman/civicrm-relationships');
}
