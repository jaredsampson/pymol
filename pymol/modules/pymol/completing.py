import cmd

expr_sc = cmd.Shortcut([
    'segi', 'chain', 'resn', 'resi', 'name', 'alt', 'elem', 'text_type',
    'formal_charge', 'numeric_type', 'ID',
    'q', 'b', 'partial_charge', 'vdw',
])

def vol_ramp_sc():
    from . import colorramping
    return cmd.Shortcut(colorramping.namedramps)

aa_exp_e = [ expr_sc                    , 'expression'      , ''   ]
aa_sel_e = [ cmd.selection_sc           , 'selection'       , ''   ]
aa_sel_c = [ cmd.selection_sc           , 'selection'       , ', ' ]
aa_obj_e = [ cmd.object_sc              , 'object'          , ''   ]
aa_obj_s = [ cmd.object_sc              , 'object'          , ' '  ]
aa_obj_c = [ cmd.object_sc              , 'object'          , ', ' ]
aa_set_c = [ cmd.setting.setting_sc     , 'setting'         , ', ' ]
aa_map_c = [ cmd.map_sc                 , 'map object'      , ', ' ]
aa_rep_c = [ cmd.repres_sc              , 'representation'  , ', ' ]
aa_v_r_c = [ vol_ramp_sc                , 'volume ramp'     , ', ' ]

def wizard_sc():
    import os, pymol.wizard
    names_glob = [name[:-3] for p in pymol.wizard.__path__
            for name in os.listdir(p) if name.endswith('.py')]
    return cmd.Shortcut(names_glob)

def get_auto_arg_list(self_cmd=cmd):
    aa_vol_c = [ lambda:
            cmd.Shortcut(self_cmd.get_names_of_type('object:volume')),
            'volume', '' ]

    return [
# 1st
        {
        'align'          : aa_sel_c,
        'alignto'        : aa_obj_c,
        'alter'          : aa_sel_e,
        'bond'           : aa_sel_e,
        'as'             : aa_rep_c,
        'bg_color'       : [ lambda c=self_cmd:c._get_color_sc(c), 'color'       , ''   ],      
        'button'         : [ self_cmd.controlling.button_sc  , 'button'          , ', ' ],
        'cartoon'        : [ self_cmd.viewing.cartoon_sc     , 'cartoon'         , ', ' ],
        'cache'          : [ self_cmd.exporting.cache_action_sc , 'cache mode'   , ', ' ],
        'center'         : aa_sel_e,
        'cealign'        : aa_sel_e,
        'color'          : [ lambda c=self_cmd:c._get_color_sc(c), 'color'       , ', ' ],
        'config_mouse'   : [ self_cmd.controlling.ring_dict_sc, 'mouse cycle'    , ''   ],
        'clean'          : aa_sel_c,
        'clip'           : [ self_cmd.viewing.clip_action_sc , 'clipping action' , ', ' ],
        'count_atoms'    : aa_sel_e,
        'delete'         : aa_sel_e,
        'deprotect'      : aa_sel_e,
        'disable'        : aa_obj_s,
        'distance'       : aa_obj_e,
        'dss'            : aa_sel_e,
        'enable'         : aa_obj_s,
        'extract'        : aa_obj_e,
        'feedback'       : [ self_cmd.fb_action_sc           , 'action'          , ', ' ],
        'fit'            : aa_sel_e,
        'flag'           : [ self_cmd.editing.flag_sc        , 'flag'            , ', ' ],
        'full_screen'    : [ self_cmd.toggle_sc              , 'option'          , ''   ],
        'get'            : aa_set_c,
        'get_area'       : aa_sel_e,
        'get_bond'       : aa_set_c,
        'gradient'       : [ self_cmd.object_sc              , 'gradient'        , ', ' ],
        'group'          : [ self_cmd.group_sc               , 'group object'    , ', ' ],
        'help'           : [ self_cmd.help_sc                , 'selection'       , ''   ],
        'hide'           : aa_rep_c,
        'isolevel'       : [ self_cmd.contour_sc             , 'contour'         , ', ' ],
        'iterate'        : aa_sel_e,
        'iterate_state'  : aa_sel_e,
        'indicate'       : aa_sel_e,
        'intra_fit'      : aa_sel_e,
        'label'          : aa_sel_e,
        'map_set'        : aa_map_c,
        'mask'           : aa_sel_e,
        'mview'          : [ self_cmd.moving.mview_action_sc , 'action'          , ''   ],
        'map_double'     : aa_map_c,
        'map_halve'      : aa_map_c,
        'map_trim'       : aa_map_c,
        'matrix_copy'    : aa_obj_c,
        'matrix_reset'   : aa_obj_c,
        'mse2met'        : aa_sel_e,
        'order'          : aa_obj_s,
        'origin'         : aa_sel_e,
        'pair_fit'       : aa_sel_c,
        'protect'        : aa_sel_e,
        'pseudoatom'     : aa_obj_c,
        'ramp_new'       : [ self_cmd.object_sc              , 'ramp'            , ', ' ],
        'reference'      : [ self_cmd.editing.ref_action_sc  , 'action'          , ', ' ],
        'remove'         : aa_sel_e,
        'reinitialize'   : [ self_cmd.commanding.reinit_sc   , 'option'          , ''   ],
        'scene'          : [ self_cmd._pymol._scene_dict_sc  , 'scene'           , ''   ],
        'sculpt_activate': aa_obj_e,
        'set'            : aa_set_c,
        'set_bond'       : aa_set_c,
        'set_name'       : aa_obj_c,
        'show'           : aa_rep_c,
        'smooth'         : aa_sel_e,
        'space'          : [ self_cmd.space_sc               , 'space'           , ''   ],      
        'spectrum'       : aa_exp_e,
        'split_chains'   : aa_sel_e,
        'split_states'   : aa_obj_c,
        'super'          : aa_sel_c,
        'stereo'         : [ self_cmd.stereo_sc              , 'option'          , ''   ],      
        'symmetry_copy'  : aa_obj_c,
        'unmask'         : aa_sel_e,
        'unset'          : aa_set_c,
        'unset_bond'     : aa_set_c,
        'update'         : aa_sel_e,
        'valence'        : [ self_cmd.editing.order_sc       , 'order'           , ', ' ],
        'volume_color'   : aa_vol_c,
        'volume_panel'   : aa_vol_c,
        'view'           : [ self_cmd._pymol._view_dict_sc   , 'view'            , ''   ],         
        'window'         : [ self_cmd.window_sc              , 'action'          , ', ' ],      
        'wizard'         : [ wizard_sc                       , 'wizard'          , ', '   ],
        'zoom'           : aa_sel_e,
        },
# 2nd
        {
        'align'          : aa_sel_e,
        'alter'          : aa_exp_e,
        'as'             : aa_sel_e,
        'bond'           : aa_sel_e,
        'button'         : [ self_cmd.controlling.but_mod_sc , 'modifier'        , ', ' ],
        'cache'          : [ self_cmd._pymol._scene_dict_sc  , 'scene'           , ''   ],
        'cealign'        : aa_sel_e,
        'color'          : aa_sel_e,
        'create'         : aa_sel_c,
        'distance'       : aa_sel_e,
        'extract'        : aa_sel_e,
        'feedback'       : [ self_cmd.fb_module_sc           , 'module'          , ', ' ],
        'flag'           : aa_sel_c,
        'get'            : aa_obj_c,
        'get_bond'       : aa_sel_c,
        'gradient'       : aa_map_c,
        'group'          : aa_obj_c,
        'hide'           : aa_sel_e,
        'isomesh'        : aa_map_c,
        'isosurface'     : aa_map_c,
        'iterate'        : aa_exp_e,
        'volume'         : aa_map_c,
        'select'         : aa_sel_e,
        'save'           : aa_sel_c,
        'load'           : aa_sel_c,
        'load_traj'      : aa_sel_c,
        'map_set'        : [ self_cmd.editing.map_op_sc      , 'operator'        , ', ' ],
        'map_new'        : [ self_cmd.creating.map_type_sc   , 'map type'        , ', ' ],
        'map_trim'       : aa_sel_c,
        'morph'          : aa_sel_e,
        'matrix_copy'    : aa_obj_c,
        'order'          : [ self_cmd.boolean_sc             , 'sort'            , ', ' ],
        'pair_fit'       : aa_sel_c,
        'reference'      : aa_sel_c,
        'scene'          : [ self_cmd.viewing.scene_action_sc, 'scene action'    , ', ' ],
        'set_name'       : [ self_cmd.selection_sc     ,       'name'            , ''   ],
        'show'           : aa_sel_e,
        'slice_new'      : aa_map_c,
        'spectrum'       : [ self_cmd.palette_sc             , 'palette'         , ''   ],      
        'super'          : aa_sel_e,
        'symexp'         : aa_obj_c,
        'symmetry_copy'  : aa_obj_c,
        'view'           : [ self_cmd.viewing.view_sc        , 'view action'     , ''   ],
        'unset'          : aa_sel_c,
        'unset_bond'     : aa_sel_c,
        'update'         : aa_sel_e,
        'ramp_new'       : aa_map_c,
        'valence'        : aa_sel_c,
        'volume_color'   : aa_v_r_c,
        },
#3rd
        {
        'button'         : [ self_cmd.controlling.but_act_sc , 'button action'   , ''   ],
        'distance'       : aa_sel_e,
        'feedback'       : [ self_cmd.fb_mask_sc             , 'mask'            , ''   ],            
        'flag'           : [ self_cmd.editing.flag_action_sc , 'flag action'     , ''   ],
        'get_bond'       : aa_sel_e,
        'group'          : [ self_cmd.creating.group_action_sc, 'group action'    , ''   ],
        'map_set'        : [ self_cmd.map_sc                 , 'map'             , ' '  ],
        'morph'          : aa_sel_e,
        'order'          : [ self_cmd.controlling.location_sc, 'location'        , ', ' ],
        'set'            : aa_sel_c,
        'set_bond'       : aa_sel_c,
        'spectrum'       : aa_sel_e,
        'symexp'         : aa_sel_c,
        'unset_bond'     : aa_sel_c,
        'valence'        : aa_sel_c,
        'volume'         : aa_v_r_c,
        },
#4th
        {
        'ramp_new'       : [ self_cmd.creating.ramp_spectrum_sc , 'ramp color spectrum' , ', ' ],      
        'map_new'        : aa_sel_c,
        'isosurface'     : aa_sel_c,
        'volume'         : aa_sel_c,
        'isomesh'        : aa_sel_c,
        'set_bond'       : aa_sel_c,
        'valence'        : aa_sel_c,
        }
        ]
