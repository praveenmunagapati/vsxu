class module_render_gl_ortho_2d : public vsx_module
{
  // in
  GLfloat tmpMat[16];

  vsx_module_param_float3* scale;

  vsx_module_param_render* render_in;
  // out
  vsx_module_param_render* render_result;

  // internal
  vsx_gl_state* gl_state;

public:

void module_info(vsx_module_specification* info)
{
  info->identifier =
    "renderers;opengl_modifiers;gl_ortho2d";

  info->description =
    "";

  info->in_param_spec =
    "render_in:render,"
    "scale:float3"
  ;

  info->out_param_spec =
    "render_out:render";

  info->component_class =
    "render";

  info->tunnel = true; // always run this
}


void declare_params(vsx_module_param_list& in_parameters, vsx_module_param_list& out_parameters)
{
  loading_done = true;
  scale = (vsx_module_param_float3*)in_parameters.create(VSX_MODULE_PARAM_ID_FLOAT3,"scale");

  scale->set(1,0);
  scale->set(1,1);
  scale->set(1,2);

	render_in = (vsx_module_param_render*)in_parameters.create(VSX_MODULE_PARAM_ID_RENDER,"render_in");
  render_in->run_activate_offscreen = true;

	render_result = (vsx_module_param_render*)out_parameters.create(VSX_MODULE_PARAM_ID_RENDER,"render_out");

  gl_state = vsx_gl_state::get_instance();
}

bool activate_offscreen() {
  // save current matrix
  gl_state->matrix_get_v( VSX_GL_PROJECTION_MATRIX, tmpMat );
  gl_state->matrix_mode( VSX_GL_PROJECTION_MATRIX );
  gl_state->matrix_glu_ortho_2d(0,1,0,1);
	return true;
}

void deactivate_offscreen() {
  // reset the matrix to previous value
  gl_state->matrix_mode( VSX_GL_PROJECTION_MATRIX );
  gl_state->matrix_load_identity();
  gl_state->matrix_mult_f( tmpMat );
}

void output(vsx_module_param_abs* param) { VSX_UNUSED(param);
  render_result->set(render_in->get());
}
};


