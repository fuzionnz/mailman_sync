<?php

function mailman_sync_role_mailman_sync_group_info() {
  $groups = array();

  // Create role based groups
  $roles = user_roles(TRUE);
  foreach ($roles as $rid => $role) {
    $groups["drupal_role_{$rid}"] = array(
      'title' => "Drupal role: $role",
      'members callback' => '_mailman_sync_role_drupal_role',
      'members arguments' => array($rid),
    );
  }

  return $groups;
}

function _mailman_sync_role_drupal_role($rid) {
  // The authenticated user role is not recorded in the user_roles table; it is just assumed for all users.
  if ($rid == 2) {
    $query = "SELECT u.name as name, u.mail as email FROM users u WHERE u.uid <> 0 AND u.mail IS NOT NULL AND u.mail <> ''";
  }
  else {
    $query = "SELECT u.name as name, u.mail as email FROM users u INNER JOIN users_roles ur ON u.uid = ur.uid WHERE ur.rid = :rid AND u.mail IS NOT NULL AND u.mail <> ''";
  }

  $result = db_query($query, array(':rid' => $rid));
  $members = array();
  foreach ($result as $row) {
    $members[$row->email] = array(
      'name' => $row->name,
      'email' => $row->email,
    );
  }
  return $members;
}
