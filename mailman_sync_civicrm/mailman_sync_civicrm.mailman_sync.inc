<?php

function mailman_sync_civicrm_mailman_sync_group_info() {
  $groups = array();

  // Load up CiviCRM.
  if (!civicrm_initialize()) {
    return $groups; // Return empty groups array.
  }

  // Grab array of all Civicrm groups.
  $civicrm_groups = civicrm_api('Group', 'get', array(
    'version' => 3,
    'sequential' => 1,
    'options' => array(
      'limit' => 0, // No limit
    ),
  ));
  if ($civicrm_groups['is_error']) {
    return $groups; // Return empty groups array.
  }

  foreach ($civicrm_groups['values'] as $civicrm_group) {
    // Remove smart groups from the list.
    if (!empty($civicrm_group['saved_search_id'])) { continue; }

    $groups["civicrm_group_{$civicrm_group['id']}"] = array(
      'title' => "Civicrm group: {$civicrm_group['name']}",
      'members callback' => '_mailman_sync_civicrm_group_members',
      'members arguments' => array($civicrm_group['id']),
    );
  }

  // Grab array of all Civicrm relationships
  $results = db_query("SELECT id, relationship_type_id, contact_b FROM {mailman_sync_civicrm}")->fetchAll();
  $relationship_types = _mailman_sync_civicrm_get_relationship_types();

  foreach($results as $result) {
    if ($result->contact_b) {
      $c = _mailman_sync_civicrm_get_contact($result->contact_b);
      $contact_name = $c['display_name'];
    }
    else {
      $contact_name = '*';
    }

    $groups["civicrm_relationship_{$result->id}"] = array(
      'title' => "Civicrm relationship: {$relationship_types[$result->relationship_type_id]} ({$contact_name})",
      'members callback' => '_mailman_sync_civicrm_relationship_members',
      'members arguments' => array($result->relationship_type_id, $result->contact_b),
    );
  }

  return $groups;
}

function _mailman_sync_civicrm_group_members($id) {
  // Load up CiviCRM.
  if (!civicrm_initialize()) {
    return array();
  }

  $group_contacts = civicrm_api('GroupContact', 'get', array(
    'version' => 3,
    'sequential' => 1,
    'group_id' => $id,
    'api.contact.getsingle' => array(
      'api.email.getsingle' => array(
        'is_primary' => 1,
      ),
    ),
  ));

  if ($group_contacts['is_error']) {
    return array();
  }

  $members = array();
  foreach ($group_contacts['values'] as $contact) {
    if (empty($contact['api.contact.getsingle']['api.email.getsingle']['email'])) {
      continue;
    }

    $members[$contact['api.contact.getsingle']['api.email.getsingle']['email']] = array(
      'name' => $contact['api.contact.getsingle']['display_name'],
      'email' => $contact['api.contact.getsingle']['api.email.getsingle']['email'],
    );
  }

  return $members;
}

function _mailman_sync_civicrm_relationship_members($relationship_type_id, $contact_b) {
  if (!civicrm_initialize()) {
    return array();
  }

  $values = array(
    'relationship_type_id' => $relationship_type_id,
    'is_active' => TRUE,
    'version' => 3,
    'options' => array(
      'limit' => 500,
    ),
    'api.contact.getsingle' => array(
      'contact_id' => '$value.contact_id_a',
      'api.email.getsingle' => array(
        'is_primary' => 1,
      ),
    ),
  );
  // Not all mappings have a contact_b field.
  if ($contact_b) {
    $values['contact_id_b'] = $contact_b;
  }
  $relationships = civicrm_api('Relationship', 'get', $values);
  
  $members = array();
  if (empty($relationships['is_error'])) {
    foreach ($relationships['values'] as $relationship) {
      $name = $relationship['api.contact.getsingle']['display_name'];
      $email = $relationship['api.contact.getsingle']['api.email.getsingle']['email'];

      $members[$email] = array(
        'name' => $name,
        'email' => $email,
      );
    }
  }
  return $members;
}
