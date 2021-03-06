<?php 

/**
 * @file
 * Helper functions for the Alpha base theme.
 */

/**
 * Invokes a preprocess or process hook in all base themes aswell 
 * as the subtheme (in that order) by including the corresponding 
 * .inc file and calling the associated function.
 * 
 * @param $type
 * 	 The type of the hook. Can be preprocess or process.
 * 
 * @param $hook
 * 	 The name of the hook.
 * 
 * @param &$vars
 *   An array of variables belonging to the (pre)process hook.
 *   Handed by reference.
 */
function alpha_invoke($type, $hook, &$vars) {
  // Fetch the hook registry from cache or static (if already populated).
  $registry = &drupal_static(__FUNCTION__);
  if (!isset($registry)) {
    $registry = variable_get('theme_' . $GLOBALS['theme_key'] . '_alpha_registry', array());
  }

  // If one of the themes in the theme trail implements this hook
  // include the corresponding .inc file and call the associated function.
  if (!empty($registry[$type][$hook])) {    
    foreach ($registry[$type][$hook] as $item) {
      if (is_file($item['path'])) {
        include_once $item['path'];      
       
        if (function_exists($item['function'])) {
          $item['function']($vars);
        }
      }
    }
  }
}

/**
 * Builds the full theme trail (deepest base theme first, subtheme last)
 * for a theme.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 to retrieve the theme trail for.
 * 
 * @param $hook
 * 	 The name of the hook.
 * 
 * @return 
 * 	 An array of all themes in the trail, keyed by theme key.
 */
function alpha_theme_trail($key) {
  $static = &drupal_static(__FUNCTION__);
  
  if (!isset($static)) {
    $themes = list_themes();
    
    if (isset($themes[$key]->info['base theme'])) {
      foreach (system_find_base_themes($themes, $key) as $base => $name) {
        if ($name && isset($themes[$base])) {
          $static[$key][$base] = $themes[$base];
        }
      }
    }

    // Add our current subtheme ($key) to that array.
    $static[$key][$key] = $themes[$key];
  }

  return $static[$key];
}

/**
 * Builds the hook registry tree for a theme and its ancestors
 * and saves it as a variable.
 * 
 * @param $name
 * 	 The key (machin-readable name) of the theme that you want
 *	 to build the registry for.
 * 
 * @param $registry
 * 	 The theme registry as an array.
 * 
 * @see 
 * 	 hook_theme_registry_alter().
 */
function alpha_build_registry($name, $registry) {
  $cache = array();
  
  // Loop through all items in the registry items and check for
  // existing (pre)process hooks in our theme trail.
  foreach ($registry as $item => $info) {
    foreach (array('preprocess', 'process') as $type) {
      $key = $type . ' functions';
     
      // Only process this array if it has hooks of this $type.
      if (!empty($info[$key])) {
        foreach (alpha_theme_trail($name) as $theme => $data) {
          $file = $type . '-' . str_replace('_', '-', $item) . '.inc';    
          $path = DRUPAL_ROOT . '/' . drupal_get_path('theme', $theme) . '/' . $type . '/' . $file;

          // If the corresponding file exists add it to the registry.
          if (is_file($path)) {
            $cache[$type][$item][] = array(
              'type' => $type,
              'hook' => $item,
              'theme' => $theme,
              'path' => $path,
              'function' => $theme . '_alpha_' . $type . '_' . $item,
            );
          }
        }
      }
    }
  }
  
  // Save the registry in the database.
  variable_set('theme_' . $name . '_alpha_registry', $cache);
}

/**
 * A wrapper function for theme_get_settings().
 * 
 * @param $name
 * 	 The name of the setting that you want to retrieve.
 * 
 * @param $theme (optional)
 * 	 The name (key) of the theme that you want to fetch the
 *   setting for. Defaults to the global variable $theme_key.
 *   
 * @param $default (optional)
 * 	 The name (key) of the theme that you want to fetch the
 *   setting for. Defaults to NULL.
 *   
 * @see 
 * 	 theme_get_setting().
 */
function alpha_theme_get_setting($name, $theme = NULL, $default = NULL) {
  $setting = theme_get_setting($name, $theme);
  
  return isset($setting) ? $setting : $default; 
}

/**
 * A helper function to return a proper options array for a form.
 * 
 * @param $start
 * 	 The number to start with.
 * 
 * @param $end
 * 	 The number to end with.
 *   
 * @param $step
 * 	 The size of a step.
 *   
 * @return 
 * 	 An array of scale options.
 */
function alpha_scale_options($start, $end, $step) {
  $options = array();  
  foreach (range($start, $end, $step) as $number) {
    // Format the value to display with one decimal.
    $options[(string) $number] = number_format($number, 1);
  }
  
  return $options;
}

/**
 * A helper function to return a proper options array for a form.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 to build the registry for.
 * 
 * @param $responsive
 * 	 A boolean indicating wether the array should cover responsive
 * 	 or non-responsive stylesheets.
 *   
 * @return 
 * 	 An array of optional or responsive stylesheet options.
 */
function alpha_css_options($key, $responsive = FALSE) {
  $items = alpha_css($key, $responsive);
  
  $output = array();
  if (!empty($items)) { 
    foreach ($items as $key => $info) {
      if ($responsive) {
        $output[$key] = '<strong>' . check_plain($info['name']) . '</strong> - ' . $info['file'] . '<div class="description">' . $info['description'] . '</div>';
      }
      else {
        $media = isset($info['options']['media']) ? $info['options']['media'] : 'all';
        $output[$key] = '<strong>' . check_plain($info['name']) . '</strong> (' . $media . ') - ' . $info['file'] . '<div class="description">' . $info['description'] . '</div>';
      }
    }
  }
  
  return $output;
}

/**
 * A helper function to return a proper options array for a form.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 to build the registry for.
 * 
 * @see
 *   hook_css_alter().
 *   
 * @return 
 * 	 An array of stylesheets that can be disabled / excluded with
 *   hook_css_alter().
 */
function alpha_exclude_options($key) {
  $items = alpha_excludes($key);
  
  $output = array(); 
  foreach ($items as $key => $info) {
    if ($info['type'] == 'exclude') {
      $output[$key] = '<strong>' . basename($key) . '</strong> - ' . t('Defined by') . ' ' . $info['name'] . '<div class="description">' . $info['description'] . '</div>';
    }
    else {
      $output[$key] = '<strong>' . basename($key) . '</strong> (' . $info['media'] . ') - ' . t('Belongs to') . ' ' . $info['name'] . '<div class="description">' . $info['description'] . '</div>';
    }
  }
  
  return $output;
}

/**
 * A helper function to return a proper options array for a form.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 to build the registry for.
 *   
 * @return 
 * 	 An array of available grids.
 */
function alpha_grid_options($key) {
  $items = alpha_grids($key);
  
  $output = array();
  if (!empty($items)) {
    foreach ($items as $key => $info) {
      $output[$key] = check_plain($info['name']);
    }
  }
    
  return $output;
}

/**
 * @todo
 */
function alpha_library_options($key) {
  $items = alpha_libraries($key);
  
  $output = array();
  if (!empty($items)) {
    foreach ($items as $key => $info) {
      $output[$key] = check_plain($info['name']) . '<div class="description">' . $info['description'] . '</div>';
    }
  }
  
  return $output;
}

/**
 * A helper function to return a proper options array for a form.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 to build the registry for.
 *   
 * @return 
 * 	 An array of available zones.
 */
function alpha_zone_options($key) {
  $items = alpha_zones($key);
  
  $output = array();
  if (!empty($items)) {
    foreach ($items as $key => $info) {
      $output[$key] = check_plain($info['name']);
    }
  }
  
  return $output;
}

/**
 * A helper function to return a proper options array for a form.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 fetch the available containers for.
 *
 * @param $grid
 * 	 The grid that you want to fetch the available containers for.
 *   
 * @return 
 * 	 An array of available containers.
 */
function alpha_container_options($key, $grid) {
  $grids = alpha_grids($key, $grid);
  
  $output = array();
  if (!empty($grids['columns'])) {
    foreach ($grids['columns'] as $count => $path) {
      $output[$count] = t('@count columns', array('@count' => $count));
    }
  }
  
  return $output;
}

/**
 * A helper function to return a proper options array for a form.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 fetch the available containers for.
 *   
 * @return 
 * 	 An array of available regions.
 */
function alpha_region_options($key) {
  $regions = alpha_regions($key);
  
  $output = array();
  if (!empty($regions)) {
    foreach ($regions as $region => $item) {
      $output[$region] = $item['name'];
    }
  }
  
  return $output;
}

/**
 * A helper function to return a proper options array for a form.
 * 
 * @param $max
 * 	 The maximum number of columns that you want to cover.
 *   
 * @return 
 * 	 An array of available columns counts.
 */
function alpha_column_options($max = NULL) {
  $output = array();
  
  if (isset($max)) {
    foreach (range(0, $max) as $width) {
      $output[$width] = t('@width columns', array('@width' => $width));
    }
  }
  
  return $output;
}

/**
 * @todo
 */
function alpha_css($key, $responsive = FALSE) {
  $css = &drupal_static(__FUNCTION__);
  
  if (!isset($css[$key])) {
    $css[$key] = variable_get('theme_' . $key . '_alpha_css');
  }

  if (!isset($css[$key])) {
    $css[$key] = alpha_register_css($key, $responsive);
  }
  
  if (isset($responsive) && isset($css[$key][$responsive])) {
    return $css[$key][$responsive];
  } 
  else if (!isset($responsive)) {
    return $css[$key];
  }
}

/**
 * @todo
 */
function alpha_register_css($key) {
  $cache = array();  

  foreach (alpha_info($key, 'css') as $theme => $data) {
    foreach ($data as $name => $info) {
      $responsive = isset($info['options']['media']) && $info['options']['media'] == 'responsive';
      $file = str_replace('_', '-', $name) . '.css';
      
      if ($responsive) {
        unset($info['options']['media']);
        unset($info['options']['weight']);
      }
      else if (isset($info['options']['weight']))  {
        $weight = $info['options']['weight'];
        $info['options']['weight'] = $weight >= 0 ? $weight : $weight - 3;
      }
      
      $cache[$key][$responsive][$name] = array(
        'name' => $info['name'],
        'description' => isset($info['description']) ? $info['description'] : '',
        'file' => $file,            
        'path' => drupal_get_path('theme', $theme) . '/css/' . $file,
        'options' => isset($info['options']) ? $info['options'] : array(),
      );
    }
  }

  if (!empty($cache[$key])) {
    variable_set('theme_' . $key . '_alpha_css', $cache[$key]);
    
    return $cache[$key];
  }
}

/**
 * @todo
 */
function alpha_libraries($key, $library = NULL) {
  $libraries = &drupal_static(__FUNCTION__);
  
  if (!isset($libraries[$key])) {
    $libraries[$key] = variable_get('theme_' . $key . '_alpha_libraries');
  }

  if (!isset($libraries[$key])) {
    $libraries[$key] = alpha_register_libraries($key, $library);
  }
  
  if (isset($library) && isset($libraries[$key][$library])) {
    return $libraries[$key][$library];
  } 
  else if (!isset($library)) {
    return $libraries[$key];
  }
}

/**
 * @todo
 */
function alpha_register_libraries($key) {
  $cache = array();  

  foreach (alpha_info($key, 'libraries') as $theme => $data) {
    foreach ($data as $name => $info) {
      $cache[$key][$name] = array(
        'name' => $info['name'],
        'description' => isset($info['description']) ? $info['description'] : '',
      );
      
      foreach (array('css', 'js') as $type) {
        if (!empty($info[$type])) {
          foreach ($info[$type] as $item) {
            $file = str_replace('_', '-', $item['file']);
            
            $cache[$key][$name][$type][] = array(
              'path' => drupal_get_path('theme', $theme) . '/' . $type . '/' . $file,
          	  'options' => isset($item['options']) ? $item['options'] : array(),
            );
          }
        }
      }
    }
  }
  
  if (!empty($cache[$key])) {
    variable_set('theme_' . $key . '_alpha_libraries', $cache[$key]);
    
    return $cache[$key];
  }
}

/**
 * Adds a column layout for a grid to your site.
 * 
 * @param $name
 * 	 A valid grid name.
 *
 * @param $columns
 * 	 The number of columns.
 */
function alpha_include_grid($name, $columns) {
  $included = &drupal_static(__FUNCTION__);
  
  if (!isset($included[$name][$columns]) || !$included[$name][$columns]) {
    // Set this name/columns combination to TRUE in the static so
    // we don't try to include it over and over.
    $included[$name][$columns] = TRUE;
    
    $grid = alpha_grids($GLOBALS['theme_key'], $name);
    $settings = alpha_settings($GLOBALS['theme_key']);
    $css = alpha_css($GLOBALS['theme_key'], TRUE);
    
    $name = str_replace('_', '-', $name);
    
    // Only go further if the grid and the desired column count actually exist.
    if (!empty($grid) && isset($grid['columns'][$columns])) {
      $path = $grid['columns'][$columns];

      // If the grid is a fluid grid we just include the corresponding column count layout.
      if ($grid['type'] == 'fluid') {
        drupal_add_css($path . '/' . $name . '-grid.css');
        drupal_add_css($path . '/columns/' . $name . '-' . $columns . '.css');
      }
      // If the grid is a fixed grid we need to determine wether we are in a responsive grid  environment
      // or not and add the grid stylesheets accordingly.
      else {
        // Un-responsive grid environments don't only require the "normal" grid layout.
        if (!$settings['responsive']['enabled']) {
          drupal_add_css($path . '/normal/' . $name . '-normal-grid.css', array('weight' => -1));
          drupal_add_css($path . '/normal/' . $name . '-normal-' . $columns . '.css', array('weight' => -1));
        }
        else {
          $widths = array('narrow' => -3, 'normal' => -2, 'wide' => -1);
          
          // Loop through all possible grid layouts (narrow, normal, wide) and add          
          // all the enabled layouts and the attached optional stylesheets via drupal_add_css().
          foreach ($widths as $type => $weight) {
            if ($settings['responsive'][$type]['enabled']) {
              $media = $settings['responsive']['enabled'] ? $settings['responsive'][$type]['media'] : 'all';
              $options = array('media' => $media, 'weight' => $weight);
              
              drupal_add_css($path . '/' . $type . '/' . $name . '-' . $type . '-grid.css', $options);
              drupal_add_css($path . '/' . $type . '/' . $name . '-' . $type . '-' . $columns . '.css', $options);
              
              // Add all enabled optional stylesheets for this responsive layout via drupal_add_css().
              foreach (array_keys(array_filter($settings['responsive'][$type]['css'])) as $item) {              
                if (isset($css[$item])) {
                  drupal_add_css($css[$item]['path'], array_merge($css[$item]['options'], $options));
                }
              }
            }        
          }
        }
      }
    }
  }
}

/**
 * A helper function for retrieving zone settings.
 * 
 * @param $name
 * 	 The name of the setting that you want to retrieve.
 * 
 * @param $zone
 * 	 The zone that you want to fetch the setting for.
 * 
 * @param $theme (optional)
 * 	 The name (key) of the theme that you want to fetch the
 *   setting for. Defaults to the global variable $theme_key.
 *   
 * @param $default (optional)
 * 	 The name (key) of the theme that you want to fetch the
 *   setting for. Defaults to NULL.
 *   
 * @see 
 * 	 alpha_theme_get_setting().
 * 
 * @see
 * 	 theme_get_setting().
 */
function alpha_zone_get_setting($name, $zone, $theme = NULL, $default = NULL) {
  return alpha_theme_get_setting('alpha_zone_' . $zone . '_' . $name, $theme, $default);
}

/**
 * A helper function for retrieving region settings.
 * 
 * @param $name
 * 	 The name of the setting that you want to retrieve.
 * 
 * @param $region
 * 	 The region that you want to fetch the setting for.
 * 
 * @param $theme (optional)
 * 	 The name (key) of the theme that you want to fetch the
 *   setting for. Defaults to the global variable $theme_key.
 *   
 * @param $default (optional)
 * 	 The name (key) of the theme that you want to fetch the
 *   setting for. Defaults to NULL.
 *   
 * @see 
 * 	 alpha_theme_get_setting().
 * 
 * @see
 * 	 theme_get_setting().
 */
function alpha_region_get_setting($name, $region, $theme = NULL, $default = NULL) {
  return alpha_theme_get_setting('alpha_region_' . $region . '_' . $name, $theme, $default);
}

/**
 * Retrieves all available grids for a theme.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 fetch the available grids for.
 * 
 * @param $grid (optional)
 * 	 The name of the grid that you want to fetch. Defaults to NULL.
 * 
 * @return
 * 	 An array of grids or the grid defined by $grid.
 */
function alpha_grids($key, $grid = NULL) {
  $grids = &drupal_static(__FUNCTION__);
  
  if (!isset($grids[$key])) {
    $grids[$key] = variable_get('theme_' . $key . '_alpha_grids');
  }

  if (!isset($grids[$key])) {
    $grids[$key] = alpha_register_grids($key);
  }
  
  if (isset($grid) && isset($grids[$key][$grid])) {
    return $grids[$key][$grid];
  } 
  else if (!isset($grid)) {
    return $grids[$key];
  }
}

/**
 * Retrieves all available grids for a theme and caches them in a 
 * variable.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 fetch the available grids for.
 */
function alpha_register_grids($key) {
  $cache = array();

  foreach (alpha_info($key, 'grids') as $theme => $data) {
    foreach ($data as $name => $info) {
      $path = drupal_get_path('theme', $theme) . '/css/grid/' . $name;

      if (!isset($cache[$key][$name])) {
        $cache[$key][$name] = array(
          'name' => $info['name'],
          'type' => isset($info['type']) && $info['type'] == 'fluid' ? 'fluid' : 'fixed',
          'path' => $path,
          'columns' => array(),
        );
      }  
      
      foreach ($info['columns'] as $column) {
        $cache[$key][$name]['columns'][$column] = $path;
      }   
    }
  }
  
  if (isset($cache[$key])) {
    variable_set('theme_' . $key . '_alpha_grids', $cache[$key]);
    
    return $cache[$key];
  }
}

/**
 * Retrieves all available zones for a theme.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 fetch the available grids for.
 *
 * @return
 *   An array of zones and their settings.
 */
function alpha_zones($key) {
  $output = &drupal_static(__FUNCTION__);
  
  if (!isset($output[$key])) {
    $themes = list_themes();
    $sections = alpha_sections();
    
    $output[$key] = array();
    if (isset($themes[$key]->info['zones'])) { 
      foreach ($themes[$key]->info['zones'] as $zone => $name) {
        $section = alpha_zone_get_setting('section', $zone, $key);
        
        $output[$key][$zone] = array(
          'zone' => $zone,
          'name' => $name,
          'enabled' => isset($sections[$section]),
          'force' => alpha_zone_get_setting('force', $zone, $key, FALSE),
          'columns' => alpha_zone_get_setting('columns', $zone, $key, 0),
          'section' => $section,
          'weight' => alpha_zone_get_setting('weight', $zone, $key, 0),
          'wrapper' => alpha_zone_get_setting('wrapper', $zone, $key, FALSE),
          'wrapper_css' => alpha_zone_get_setting('wrapper_css', $zone, $key),
          'primary' => alpha_zone_get_setting('primary', $zone, $key),
          'css' => alpha_zone_get_setting('css', $zone, $key),
        );
      }
    }
    
    uasort($output[$key], 'drupal_sort_weight');
  }
  
  return $output[$key];
}

/**
 * Retrieves all available regions for a theme.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 fetch the available grids for.
 *
 * @return
 *   An array of regions and their settings.
 */
function alpha_regions($key) {
  $output = &drupal_static(__FUNCTION__);
  
  if (!isset($output[$key])) {
    $regions = system_region_list($key);
    $zones = alpha_zones($key);
    $exclude = alpha_regions_exclude();
    
    $output[$key] = array();
    foreach ($regions as $region => $name) {
      if (!in_array($region, $exclude)) {
        $zone = alpha_region_get_setting('zone', $region, $key);
        $prefix = alpha_region_get_setting('prefix', $region, $key, 0);
        $columns = alpha_region_get_setting('columns', $region, $key, 0);
        $suffix = alpha_region_get_setting('suffix', $region, $key, 0);
        
        $output[$key][$region] = array(
          'region' => $region,
          'name' => $name,
          'zone' => $zone,
          'enabled' => isset($zones[$zone]),
          'force' => alpha_region_get_setting('force', $region, $key, FALSE),
          'prefix' => $prefix,
          'columns' => $columns,
          'suffix' => $suffix,
          'width' => $prefix + $columns + $suffix,
          'push' => 0,
          'pull' => 0,
          'css' => alpha_region_get_setting('css', $region, $key),
          'weight' => alpha_region_get_setting('weight', $region, $key, 0),
          'primary' => isset($zone) && isset($zones[$zone]) ? $zones[$zone]['primary'] == $region : FALSE,
        );
      }
    }
    
    uasort($output[$key], 'drupal_sort_weight');
  }
  
  return $output[$key];
}

/**
 * A helper function that holds the available sections.
 * 
 * @return
 *   The array of available sections.
 */
function alpha_sections() {
  return array(
    'header' => t('Header'),
    'content' => t('Content'),
    'footer' => t('Footer'),
  );
}

/**
 * A helper function for retrieving the content of theme .info files
 * in the theme trail of $key.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 fetch the theme trail .info content for.
 * 
 * @param $item
 *   The name of the variable that you want to fetch.
 *   
 * @return
 *   The $item content of all themes .info files in the theme trail.
 */
function alpha_info($key, $item) {
  $output = &drupal_static(__FUNCTION__);
  
  if (!isset($output[$key][$item])) {
    $output[$key][$item] = array();
    foreach (alpha_theme_trail($key) as $theme => $info) {
      if (!empty($info->info[$item])) {
        $output[$key][$item][$theme] = $info->info[$item];
      }
    }    
  }
  
  return $output[$key][$item];
}

/**
 * @todo
 */
function alpha_debug_settings($key) {
  $settings = array(
    'block' => alpha_theme_get_setting('alpha_debug_block_toggle', $key, FALSE),
    'grid' => alpha_theme_get_setting('alpha_debug_grid_toggle', $key, FALSE),
    'roles' => alpha_theme_get_setting('alpha_debug_grid_roles', $key),
  );
  
  $settings['roles'] = is_array($settings['roles']) ? $settings['roles'] : array();
  $settings['access'] = alpha_debug_access($GLOBALS['user'], $settings['roles']);

  return $settings;
}

/**
 * A helper function to gather all non-region and non-zone settings for the
 * theme defined by $key.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 fetch the settings for.
 *
 * @return
 *   A nested array of theme settings.
 */
function alpha_settings($key) {
  $settings = &drupal_static(__FUNCTION__);
  
  if (!isset($settings[$key])) {
    $settings[$key] = array(
      'grid' => alpha_theme_get_setting('alpha_grid', $key, 'default'),
      'css' => alpha_theme_get_setting('alpha_css', $key, array()),
      'libraries' => alpha_theme_get_setting('alpha_libraries', $key, array()),
      'exclude' => alpha_theme_get_setting('alpha_exclude', $key, array()),      
      'responsive' => array(
  		'enabled' => alpha_theme_get_setting('alpha_responsive', $key, FALSE),
        'narrow' => array(
          'enabled' => alpha_theme_get_setting('alpha_responsive_narrow', $key, FALSE),
          'media' => alpha_theme_get_setting('alpha_responsive_narrow_media', $key, 'all'),
          'css' => alpha_theme_get_setting('alpha_responsive_narrow_css', $key),
        ),
        'normal' => array(
          'enabled' => alpha_theme_get_setting('alpha_responsive_normal', $key, TRUE),
          'media' => alpha_theme_get_setting('alpha_responsive_normal_media', $key, 'all'),
          'css' => alpha_theme_get_setting('alpha_responsive_normal_css', $key),
        ),
        'wide' => array(
          'enabled' => alpha_theme_get_setting('alpha_responsive_wide', $key, FALSE),
          'media' => alpha_theme_get_setting('alpha_responsive_wide_media', $key, 'all'),
          'css' => alpha_theme_get_setting('alpha_responsive_wide_css', $key),
        ),
      ),
      'viewport' => array(
        'enabled' => alpha_theme_get_setting('alpha_viewport', $key, FALSE),
        'initial' => alpha_theme_get_setting('alpha_viewport_initial_scale', $key, 1),
        'min' => alpha_theme_get_setting('alpha_viewport_min_scale', $key, 1),
        'max' => alpha_theme_get_setting('alpha_viewport_max_scale', $key, 1),
        'user' => alpha_theme_get_setting('alpha_viewport_user_scaleable', $key, TRUE),
      ),
      'debug' => array(
        'block' => alpha_theme_get_setting('alpha_debug_block_toggle', $key, FALSE),
        'grid' => alpha_theme_get_setting('alpha_debug_grid_toggle', $key, FALSE),
        'roles' => alpha_theme_get_setting('alpha_debug_grid_roles', $key),
      ),
    );

    foreach (alpha_toggle() as $item => $title) {
      $settings[$key]['toggle'][$item] = alpha_theme_get_setting('alpha_toggle_' . $item, $key, TRUE);
    }
    
    foreach (alpha_visibility() as $item => $title) {
      $settings[$key]['hidden'][$item] = alpha_theme_get_setting('alpha_hidden_' . $item, $key, FALSE);
    }
    
    foreach (array('narrow', 'normal', 'wide') as $type) {
      $settings[$key]['responsive'][$type]['css'] = is_array($settings[$key]['responsive'][$type]['css']) ? $settings[$key]['responsive'][$type]['css'] : array();
    }
    
    $settings[$key]['debug']['roles'] = is_array($settings[$key]['debug']['roles']) ? $settings[$key]['debug']['roles'] : array();
    $settings[$key]['debug']['access'] = alpha_debug_access($GLOBALS['user'], $settings[$key]['debug']['roles']);
    
    alpha_alter('alpha_settings', $settings[$key], $key);
  }
  
  return $settings[$key];
}

/**
 * This function "fixes" drupal_alter so it also works in the theme-settings and anywhere else 
 * where you want to be 100% certain that drupal_alter uses the proper global $theme.
 * 
 * The problem with drupal_alter is, that it always relies on the global $theme while
 * the theme-settings page relies (and "overrides") the global $theme_key variable while
 * building its form.
 * 
 * @param $type
 * 
 * @param $data
 * 
 * @param $context1
 * 
 * @param $context2
 * 
 * @see 
 *   See drupal_alter() for more information about how this works.
 */
function alpha_alter($type, &$data, &$context1 = NULL, &$context2 = NULL) {
  global $theme, $base_theme_info;

  if ($theme != $context1) {
    $themes = list_themes();
    
    if (!empty($themes[$context1])) {
      $theme_original = $theme;
      $base_theme_info_original = $base_theme_info;

      $base_theme_info = alpha_theme_trail($context1);
      
      if (!empty($base_theme_info)) {
        foreach ($base_theme_info as $theme) {
          include_once drupal_get_path('theme', $theme->name) . '/template.php';
        }
      }
     
      array_pop($base_theme_info);
      
      $theme = $context1;
            
      drupal_alter($type, $data, $context1, $context2);      
      
      $functions = &drupal_static('drupal_alter');
      unset($functions['alpha_settings']);
      
      $theme = $theme_original;
      $base_theme_info = $base_theme_info_original;
    }
  }
  else {
    drupal_alter($type, $data, $context1, $context2);
  }
}

/**
 * A helper function to check wether the user defined by $user
 * matches one of the roles defined by $roles.
 * 
 * @param $user
 *   A Drupal user as returned by user_load().
 * 
 * @param $roles
 *   An array of roles that you want to check against $user.
 * 
 * @return
 *   A boolean, indicating wether or not $user matches one of
 *   the $roles.
 */
function alpha_debug_access($user, $roles) {
  foreach ($roles as $role => $assigned) {
    if ($assigned && isset($user->roles[$role])) {
      return TRUE;
    }
  }
  
  return FALSE;
}

/**
 * A helper function that returns an array of un-wanted Drupal core regions.
 * 
 * @return
 *   An array of un-wanted regions.
 */
function alpha_regions_exclude() {
  return array('page_top', 'page_bottom');
}

/**
 * Builds and returns a debugging block.
 * 
 * @param $region
 *   The region that the block will be placed in.
 * 
 * @param $info
 *   The region information.
 * 
 * @return
 *   A debugging block in a renderable array.
 */
function alpha_grid_block($grid) {
  $block = new stdClass();
  $block->delta = 'grid-indicator';
  $block->region = 'page_bottom';
  $block->module = 'alpha';
  $block->subject = t('Responsive grid indicator block for @name', array('@name' => $grid));
  $block->weight = -999;

  return array(
	'#printed' => FALSE,
    '#theme_wrappers' => array('block'),
    '#block' => $block,
    '#markup' => '<div class="alpha-responsive-indicator element-invisible">' . t('This is a responsive grid indicator block.') . '</div>',
    '#contextual_links' => array('test' => array('admin/structure', array($block->delta))),
  );
}

/**
 * Adds a debug grid to the site.
 * 
 * @param $settings
 *   The settings array as returned by alpha_settings().
 * 
 * @param $columns
 *   The column layout (number) that you want to add a debugging grid
 *   for.
 */
function alpha_debug_grid($settings, $columns) {
  $debug = &drupal_static(__FUNCTION__);
  $grid = alpha_grids($GLOBALS['theme_key'], $settings['grid']);
  
  if ($grid['type'] != 'fluid' && !isset($debug[$columns])) {
    $widths = array('narrow' => -3, 'normal' => -2, 'wide' => -1);
                
    foreach ($widths as $type => $weight) {
      if ($type == 'normal' || ($settings['responsive']['enabled'] && $settings['responsive'][$type]['enabled'])) {
        $media = $settings['responsive']['enabled'] ? $settings['responsive'][$type]['media'] : 'all';
        $path = $grid['columns'][$columns] . '/' . $type . '/debug-' . $columns . '.png';
        $overlay = 'body.alpha-grid-debug .container-' . $columns . ' { background-image: url(' . file_create_url($path) . '); }' . "\n";
        
        drupal_add_css($overlay, array('type' => 'inline', 'media' => $media, 'weight' => $weight));
        
        if ($settings['responsive']['enabled']) {          
          $indicator = 'body.alpha-grid-debug .block-grid-indicator { background-position: ' . $weight * 150 . 'px 0; }';
          
          drupal_add_css($indicator, array('type' => 'inline', 'media' => $media, 'weight' => $weight));
        }        
      }        
    }
  }
  
  $debug[$columns] = TRUE;
}

/**
 * Builds and returns a debugging block.
 * 
 * @param $key
 * 	 The key (machin-readable name) of the theme that you want
 *	 fetch the exclusion candidates for.
 * 
 * @return
 *   An array of files that can be excluded with hook_css_alter().
 * 
 * @see
 *   alpha_css_alter().
 * 
 * @see 
 *   hook_css_alter().
 */
function alpha_excludes($key) {
  $themes = list_themes();
  
  $styles = array();
  foreach (system_rebuild_module_data() as $module => $data) {
    if ($data->status && !empty($data->info['stylesheets'])) {
      foreach ($data->info['stylesheets'] as $media => $content) {        
        foreach ($content as $file) {          
          $styles[$file] = array(
            'type' => 'module',
            'source' => $module,
            'name' => $data->info['name'],
            'file' => $file,
            'media' => $media,
          	'description' => NULL,
          );
        }
      }
    }
  }
  
  foreach (alpha_info($key, 'stylesheets') as $theme => $data) {
    foreach ($data as $media => $content) {
      foreach ($content as $file) {
        $styles[$file] = array(
          'type' => 'theme',
          'source' => $theme,
          'name' => $themes[$theme]->info['name'],
          'file' => $file,
          'media' => $media,
          'description' => NULL,
        );
      }
    }
  }
  
  foreach (alpha_info($key, 'exclude') as $theme => $data) {
    foreach ($data as $file => $description) {
      $styles[$file] = array(
        'type' => 'exclude',
        'source' => $theme,
        'name' => $themes[$theme]->info['name'],
        'file' => $file,
        'media' => NULL,
        'description' => $description,
      );
    }
  }
  
  return $styles;
}

/**
 * @todo
 */
function alpha_toggle() {
  return array(
  	'messages' => t('Messages'), 
  	'action_links' => t('Action links'), 
  	'tabs' => t('Tabs'), 
  	'breadcrumb' => t('Breadcrumb'), 
  	'page_title' => t('Page title'), 
  	'feed_icons' => t('Feed icons'),
  );
}

/**
 * @todo
 */
function alpha_visibility() {
  return array(
  	'title' => t('Page title'), 
  	'site_name' => t('Site name'), 
  	'site_slogan' => t('Site slogan'), 
  );
}