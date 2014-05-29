<?php

function _mailman_sync_configuration_form($form_state) {
  $groups = mailman_sync_group_info_load();
  // Create groups status array
  $status = array();
  foreach ($groups as $group_id => $group) {
    $status[$group_id] = $group['status'];
  }

  // Build a select table.
  $header = array(
    'name'     => t('Name'),
    'group_id' => t('Group ID'),
    'mailman_list' => t('Mailman List'),
    'edit'         => t('Edit'),
  );
  $options = array();
  foreach ($groups as $group_id => $group) {
    if ($group['status']) {
      $members_list = l(
        t('View'),
        "mailman/sync/{$group_id}",
        array(
          'query' => array(
            'mailman_key' => variable_get('mailman_sync_key'),
          ),
        )
      );
    }
    else $members_list = t('Disabled');

    $options[$group_id] = array(
      'group_id' => $group_id,
      'name'     => $group['title'],
      'mailman_list' => $group['mailman_list'],
      'edit' => l('Edit', "admin/config/mailman/{$group_id}/edit"),
      '#disabled' => TRUE,
    );
  }

  $form = array();
  $form['#submit'] = array('_mailman_sync_configuration_submit');
  $form['mailman_mapping_link'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mapping link'),
    '#description' => t('This link provides the mapping data between group and mailman list to the bash script. It must be passed to this script in the form <code>mailman_sync.sh -u "http://mysecretlink.com/" -k secretkey</code>.'),
  );
  $form['mailman_mapping_link']['link'] = array(
    '#prefix' => '<p>',
    '#suffix' => '</p>',
    '#markup' => t("<strong>Mapping link:</strong> !mapping_link", array('!mapping_link' => url('mailman/sync/map', array('absolute' => TRUE)))),
  );
  $form['mailman_sync_key'] = array(
    '#type' => 'fieldset',
    '#title' => t('Secret key'),
    '#description' => t('This key is appended to all requests for member mappings and listings. You can regenerate the key if you need to (eg. for security reasons). Please note that you will need to update all mailman scripts to use the new key.'),
  );
  $form['mailman_sync_key']['mailman_sync_display_key'] = array(
    '#prefix' => '<p>',
    '#suffix' => '</p>',
    '#markup' => t('<strong>Secret key:</strong> !key', array('!key' => variable_get('mailman_sync_key', '<em>no key exists</em>'))),
  );
  $form['mailman_sync_key']['mailman_sync_generate_key'] = array(
    '#type' => 'button',
    '#value' => t('Generate new key'),
    '#validate' => array('_mailman_sync_configuration_validate'),
  );
  $form['mailman_sync_description'] = array(
    '#prefix' => '<p>',
    '#suffix' => '</p>',
    '#markup' => t("Please select which groups to make available to the mailman sync."),
  );
  $form['mailman_sync_groups_status'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#js_select' => FALSE,
    '#empty' => t('No groups are enabled. Please enable mailman_sync group modules.'),
    '#default_value' => $status,
  );
  $form['mailman_sync_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
  );

  return $form;
}

function _mailman_sync_configuration_validate($form, &$form_state) {
  variable_set('mailman_sync_key', _mailman_sync_generate_key());
}

function _mailman_sync_configuration_submit($form, &$form_state) {
  $groups_status = $form_state['values']['mailman_sync_groups_status'];
  foreach ($groups_status as $group_id => $group_status) {
    $record = array('group_id' => $group_id, 'status' => $group_status ? 1 : 0);
    drupal_write_record('mailman_sync', $record, 'group_id');
  }
}

function _mailman_sync_edit_form($form, &$form_state, $group) {
  $form = array();
  $form['#submit'] = array('_mailman_sync_edit_form_submit');
  $form['#validate'] = array('_mailman_sync_edit_form_validate');
  $form['group_id'] = array(
    '#type' => 'hidden',
    '#value' => $group['group_id'],
  );
  $form['title'] = array(
    '#prefix' => '<h2>',
    '#suffix' => '</h2>',
    '#markup' => "{$group['title']} ({$group['group_id']})",
  );
  $form['mailman_list'] = array(
    '#type' => 'textfield',
    '#title' => 'Mailman list name',
    '#description' => t('Enter the name of the matching mailman list for this group.'),
    '#default_value' => $group['mailman_list'],
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
  );
  return $form;
}

function _mailman_sync_edit_form_validate($form, &$form_state) {
}

function _mailman_sync_edit_form_submit($form, &$form_state) {
  $group = mailman_sync_group_load($form_state['values']['group_id']);
  $record = array(
    'group_id' => $group['group_id'],
    'mailman_list' => $form_state['values']['mailman_list'],
    'mailman_password' => $form_state['values']['mailman_password'],
  );
  drupal_write_record('mailman_sync', $record, 'group_id');
  drupal_goto('admin/config/mailman');
}
