<?php

/*
 * Psuedo hook information for automated batch form population
 */
$batch = array(
  // (*required) - key for all functions. file name must be _custom_batch.newsletter_subscribe.inc
  'name'  => 'newsletter_unsubscribe', 
  // (optional) - title for fieldgroup on admin page
  'title' => 'Newsletter Mass Unsubscribe',
  // (optional) - description for admin page
  'description' => 'Unsubscribe all users with the "subscriber" role from the newsletter',
  // (optional) - submit button text for admin page
  'submit' => 'Start',
);

/*
 * Gather data to batch, normally a sql query
 *
 * @return array
 */
function _custom_batch_newsletter_unsubscribe_data(){
  $data = array();
  
  $sql = "SELECT u.mail FROM {users} as u LEFT JOIN {users_roles} as ur ON u.uid = ur.uid WHERE ur.rid = 13";
  $results = db_query($sql);
  while($row = db_fetch_object($results)){
    $data[] = $row;
  }
  
  return $data;
}

/*
 * Single row of batch op
 *
 * @param array $item
 *    - whatever makes up the array of items in the _data function
 */ 
function _custom_batch_newsletter_unsubscribe_row($item) {
  // do work here
	//dsm($item->mail);
	// 376 is newsletter tid
	simplenews_unsubscribe_user($item->mail, 376, FALSE, 'website');
}