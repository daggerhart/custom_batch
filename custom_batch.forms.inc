<?php


/*
 * Admin settings
 */
function custom_batch_settings_form() {
  $form = array();
  $form['custom_batch_chunk_size'] = array(
    '#type' => 'textfield',
    '#title' => 'Chunk Size',
    '#description' => 'Number of items to process for each operation.',
    '#default_value' => variable_get('custom_batch_chunk_size', 10),
    '#required' => true,
  );
  $form['custom_batch_memory_limit'] = array(
    '#type' => 'select',
    '#title' => 'Memory Limit',
    '#description' => 'Does not change memory limit during batch process if unselected.  Site current memory_limit is '.ini_get('memory_limit').'.',
    '#default_value' => variable_get('custom_batch_memory_limit', 0),
    '#options' => array(
      0      => '--default--',
      '128M' => '128M',
      '196M' => '196M',
      '256M' => '256M',
    ),
  );
  $form['custom_batch_debug'] = array(
    '#type' => 'checkbox',
    '#title' => 'Debug with watchdog statements',
    '#default_value' => variable_get('custom_batch_debug', 0),
  );
  $form['custom_batch_test_only'] = array(
    '#type' => 'checkbox',
    '#title' => 'Test Mode: does not execute batch row function.',
    '#default_value' => variable_get('custom_batch_test_only', 0),
  );
  return system_settings_form($form);
}

/**
 * Form builder function to allow choice of which batch to run.
 * FOR ALL BATCHES
 */
function custom_batch_batch_form() {
	$unique_submit = array();
	
  // loop through batchs and make a form
  if ($batches = _custom_batch_get_batches()){
		$i=0;
    foreach ($batches as $name => $details) {
			// submit values must be unique
			$i++;
			$submit_value = ($details['submit']) ? $details['submit'] : 'Process';
			if (isset($unique_submit[$submit_value])){
				// this submit value is not unique, increment it
				$submit_value = $submit_value.' - '.$i;
			}
			$unique_submit[$submit_value] = $submit_value;
			
      $form[$name] = array(
        '#type' => 'fieldset',
        '#title' => ($details['title']) ? $details['title'] : $name,
      );
      $form[$name]['description'] = array(
        '#type' => 'markup',
        '#value' => '<div class="description">'.(($details['description']) ? $details['description']: 'no description').'</div>',
      );
      $form[$name]['submit'] = array(
        '#type' => 'submit',
        '#value' => $submit_value,
      );
    }
  }
	
  return $form;
}

/*
 * submit handler FOR ALL BATCHES
 */
function custom_batch_batch_form_submit($form, &$form_state) {
  // Execute the function named custom_batch_fix_cc or custom_batch_2.
  $name = array_shift(array_values($form_state['clicked_button']['#array_parents']));

  module_load_include('inc', 'custom_batch', 'custom_batch.batch_handler');
  
  // users, regprods, or rmas
  if ($batch = _custom_batch_batch_builder($name)) {
    batch_set($batch);
  }
}

/*
 * Get batches from folder
 */
function _custom_batch_get_batches(){
  $files = file_scan_directory(drupal_get_path('module', 'custom_batch'). '/batches', '.inc');
  $batches = array();
  foreach ($files as $absolute => $file) {
    require $absolute;
    if (isset($batch) && isset($batch['name'])) {
      $batches[$batch['name']] = $batch;
    }   
  }
  return $batches;
}
