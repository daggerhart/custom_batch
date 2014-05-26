<?php

/**
 * GENERIC Batch BUILDER for all batches
 * 
 * Query data and chunk results.  Build batch
 *
 * @param $name
 *    string - batch name
 *
 *  @params $items_per_chunk
 *    int - number of items per batch op
 *    
 *  @return array
 *    - batch array
 */
function _custom_batch_batch_builder($name, $items_per_chunk = 10, $start=1, $stop=100000) {
  // include the data_type's file
  module_load_include('inc', 'custom_batch', 'batches/_custom_batch.'.$name);
  
  // make sure we have teh data function
  if (function_exists('_custom_batch_'.$name.'_data')){
    $query_function =  '_custom_batch_'.$name.'_data';
    $data = $query_function();
    
    if (variable_get('custom_batch_debug', 0)) {
      watchdog('cb_'.$name, 'Data: ' .print_r($data,1));
    }
    
    if (variable_get('custom_batch_chunk_size', 0)){
      $items_per_chunk = variable_get('custom_batch_chunk_size', $items_per_chunk);
    }
    
    //Break up all of our data so each process does not time out.
    $chunks = array_chunk($data, $items_per_chunk);
    $operations = array();
   
    //for every chunk, assign some method to run on that chunk of data
    foreach ($chunks as $chunk) {
      $operations[] = array(
        "_custom_batch_batch_operation",
        array(
          $chunk, // parameter 1 of operation
          $name, 
        ) 
      );
    }
   
    //put all that information into our batch array
    $batch = array(
      'operations' => $operations,
      'title' => t('Custom Batch: '.$name),
      'init_message' => t('Initializing'),
      'error_message' => t('An error occurred'),
      'finished' => '_custom_batch_batch_finished',
      'file' => drupal_get_path('module', 'custom_batch').'/custom_batch.batch_handler.inc',
    );
    
    return $batch;
  }
  return false;
}



/*
 * BATCH operation FOR ALL BATCHES
 * =================================================================
 */
function _custom_batch_batch_operation($chunk, $name, &$context) {
  
  module_load_include('inc', 'custom_batch', 'batches/_custom_batch.'.$name);
  
  // custom memory limit
  if (variable_get('custom_batch_memory_limit', 0)) {
    ini_set('memory_limit', variable_get('custom_batch_memory_limit', ini_get('memory_limit')));
  }
  
  // set some batch details
  if (variable_get('custom_batch_debug', 0)) {
    watchdog('cb_'.$name, 'Chunk: ' .print_r($chunk,1));
    watchdog('cb_'.$name, 'Batch type: ' .$name);
  }
  
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_item'] = 0;
    $context['sandbox']['max'] = count($chunk);
  }
  
  foreach($chunk as $i => $item){
    // auto-run specific function
    $function = "_custom_batch_".$name."_row";
    // skip if function doesn't exist or in test mode
    if (function_exists($function) && variable_get('custom_batch_test_only', 0) == 0) {
      $function($item);
    }
    
    // Update our progress information.
    $context['sandbox']['progress']++;
    $context['sandbox']['current_item'] = (is_string($item)) ? $item : $i;
    $context['message'] = t('Now processing %item', array('%item' => $context['sandbox']['current_item']));
  }

  $context['results'][] = $name. ' processed '.($i+1).' items.'; //.array_shift(array_keys($chunk)) .' -> '.array_pop(array_keys($chunk));
  
  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  
  // debugging
  if (variable_get('custom_batch_debug', 0)) {
    watchdog('cb_'.$name, '========context=======<pre>'.print_r($context,1).'</pre>');
  }
}

/*
 * Batch finish method - FOR ALL BATCHES
 */
function _custom_batch_batch_finished($success, $results, $operations) {
  if ($success) {
    // Here we do something meaningful with the results.
    $message = count($results) .' processed.';
    $message .= theme('item_list', $results);  // D6 syntax
    drupal_set_message($message);
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}
