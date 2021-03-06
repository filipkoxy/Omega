<?php

/*
 * @todo
 */
function omega_alpha_preprocess_node(&$vars) {
  // Prepare the arrays to handle the classes and ids for the node container.
  $vars['attributes_array']['id'] = drupal_html_id('node-' . $vars['type'] . '-' . $vars['nid']);  
  $vars['attributes_array']['class'] = $vars['classes_array'];
  
  // Add a class to allow styling of nodes of a specific type.
  $vars['attributes_array']['class'][] = drupal_html_class($vars['type']);
  
  // Add a class to allow styling based on publish status.
  $vars['attributes_array']['class'][] = $vars['status'] ? 'published' : 'unpublished';
  
  // Add a class to allow styling based on promotion.
  $vars['attributes_array']['class'][] = $vars['promote'] ? 'promoted' : 'not-promoted';
  
  // Add a class to allow styling based on sticky status.
  $vars['attributes_array']['class'][] = $vars['sticky'] ? 'sticky' : 'not-sticky';
  
  // Add a class to allow styling based on if a node is showing a teaser or the whole thing.
  $vars['attributes_array']['class'][] = $vars['teaser'] ? 'teaser' : 'full-view';
  
  // Add a class to allow styling of nodes being viewed by the author of the node in question.
  if ($vars['uid'] == $vars['user']->uid) {
    $vars['attributes_array']['class'][] = 'self-posted';
  }
  
  // Add a class to allow styling based on the node author.
  $vars['attributes_array']['class'][] = drupal_html_class('author-' . $vars['node']->name);
  
  // Add a class to allow styling for zebra striping.
  $vars['attributes_array']['class'][] = drupal_html_class($vars['zebra']);
  
  // Add a class to make the node container self clearing.
  $vars['attributes_array']['class'][] = 'clearfix';
  
  // Remove node classes provided by core where duplication exists.
  $exclusions = array(drupal_html_class('node-' . $vars['type']), 'node-promoted', 'node-sticky', 'node-teaser', 'node-unpublished');
  foreach ($vars['attributes_array']['class'] as $key => $value) {
    if (in_array($value, $exclusions)) {
      unset($vars['attributes_array']['class'][$key]);
    }
  }
  
  $vars['content_attributes_array']['class'] = array('content', 'clearfix');
  
  // Adding a class to the title attributes
  $vars['title_attributes_array']['class'] = 'node-title';
}