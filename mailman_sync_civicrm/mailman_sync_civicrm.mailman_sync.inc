<?php

function mailman_sync_civicrm_mailman_sync_group_info() {
  $groups = array();

  // Load up CiviCRM.
  if (!civicrm_initialize()) {
    return $groups; // Return empty groups array.
  }

  // Grab array of all Civicrm groups.
  $civicrm_groups = civicrm_api('Group', 'get', array('version' => 3, 'sequential' => 1));
  if ($civicrm_groups['is_error']) {
    return $groups; // Return empty groups array.
  }

  foreach ($civicrm_groups['values'] as $civicrm_group) {
    $groups["civicrm_group_{$civicrm_group['id']}"] = array(
      'title' => "Civicrm group: {$civicrm_group['name']}",
      'members callback' => '_mailman_sync_civicrm_group_members',
      'members arguments' => array($civicrm_group['id']),
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

    $members[] = array(
      'name' => $contact['api.contact.getsingle']['display_name'],
      'email' => $contact['api.contact.getsingle']['api.email.getsingle']['email'],
    );
  }

  return $members;
}
