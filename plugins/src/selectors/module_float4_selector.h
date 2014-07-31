/**
* Written by Alastair Cota for:
* 
* Project: VSXu: Realtime modular visual programming engine.
*
* This file is part of Vovoid VSXu.
*
* @author Jonatan Wallmander, Robert Wenzel, Vovoid Media Technologies AB Copyright (C) 2003-2014
* @see The GNU Lesser General Public License (LGPL)
*
* VSXu Engine is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU Lesser General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <string>
#include <sstream>

#define SEQ_RESOLUTION 8192

class module_float4_selector : public vsx_module
{
//---DATA-STORAGE---------------------------------------------------------------

  // in
  vsx_module_param_float* index;
  vsx_module_param_int* inputs;
  std::vector<vsx_module_param_float4*> float_x;
  vsx_module_param_int* wrap;
  vsx_module_param_int* interpolation;
  vsx_module_param_sequence* sequence;
  vsx_module_param_int* reverse;
  vsx_module_param_int* reset_seq_to_default;

  // out
  vsx_module_param_float4* result;

  // internal
  int i_prev_inputs;
  int i_curr_inputs;  

  float i_index;
  int i_index_x;
  bool i_underRange;
  bool i_overRange;

  int i_index_x0;
  int i_index_x1;
  float i_value_y0[4];
  float i_value_y1[4];
  
  int i_wrap;
  int i_interpolation;
  vsx_sequence i_sequence;
  vsx_sequence i_seq_default;
  vsx_array<float> i_sequence_data;
  long i_seq_index;
  int i_reverse;
  int i_reset_seq_to_default;
 
  std::stringstream i_paramString;
  std::stringstream i_paramName;
  vsx_string i_in_param_string;
  vsx_string i_out_param_string;
  bool i_am_ready;

//---INITIALISATION-------------------------------------------------------------

public:

  //Initialise Data
  bool init() 
  {
    i_am_ready = false;

    i_prev_inputs = 2;   //"3"
    i_curr_inputs = 2;
    
    i_index = 0.0;
    i_index_x = 0; 
    i_underRange = false;
    i_overRange = false;

    i_index_x0 = 0;
    i_index_x1 = 1;
    for(int i = 0; i < 4; i++)
    {  
      i_value_y0[i] = 0.0;
      i_value_y1[i] = 0.0;
    }
    
    i_wrap = 2;          //Wrap
    i_interpolation = 0; //No Interpolation
    i_reverse = 2;       //Autoreverse Normal
    i_reset_seq_to_default = -1;

    i_in_param_string = "";
    i_out_param_string = "";
    
    return true;
  }
  
  //Initialise Module & GUI
  void module_info(vsx_module_info* info)
  {
    info->identifier =
      "selectors;float4_selector";

    info->description =
      "[result] is equal to the\n"
      "[float4_x] chosen by [index]\n"
      "\n"
      "The number of inputs is\n"
      "controlled by [inputs]\n"
      "\n"
      "Linear and Sequence\n"
      "Interpolation options\n"
      "are available in [options]\n"
      "\n";

    info->out_param_spec =
      "result:float4";

    info->in_param_spec =
      "index:float,"
      "inputs:enum?1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16,"
      + i_in_param_string +
      "options:complex{"
        "wrap:enum?None_Zero|None_Freeze|Wrap,"
        "interpolation:enum?None|Linear|Sequence,"
        "sequence:sequence,"
        "reverse:enum?Off|On|Auto_Normal|Auto_Inverted,"
        "reset_seq_to_default:enum?ok}";

    info->component_class = "parameters";

    info->output = 1; // Always Running
  }

  //Build Interface
  void declare_params(vsx_module_param_list& in_parameters, vsx_module_param_list& out_parameters)
  {
    loading_done = true;
    redeclare_in=true;
  }

  //Rebuild Inputs
  void redeclare_in_params(vsx_module_param_list& in_parameters)
  {
    loading_done = true;

    index = (vsx_module_param_float*)in_parameters.create(VSX_MODULE_PARAM_ID_FLOAT, "index");
    index->set(i_index);
    inputs = (vsx_module_param_int*)in_parameters.create(VSX_MODULE_PARAM_ID_INT, "inputs");
    inputs->set(i_prev_inputs);
    
    float_x.clear();
    i_paramString.str("");
    i_paramString << "float_x:complex{";

    for(int i = 0; i <= i_prev_inputs; ++i)
    {
        if(i > 0) i_paramString << ",";
        i_paramName.str("");
        i_paramName << "float4_" << i;
        i_paramString << i_paramName.str().c_str() << ":float4";

        float_x.push_back((vsx_module_param_float4*)in_parameters.create(VSX_MODULE_PARAM_ID_FLOAT4, i_paramName.str().c_str()));
        float_x[i]->set(0.0, 0);
        float_x[i]->set(0.0, 1);
        float_x[i]->set(0.0, 2);
        float_x[i]->set(0.0, 3);
    }

    i_paramString << "},";
    i_in_param_string = i_paramString.str().c_str();
    
    wrap = (vsx_module_param_int*)in_parameters.create(VSX_MODULE_PARAM_ID_INT, "wrap");
    wrap->set(i_wrap);
    interpolation = (vsx_module_param_int*)in_parameters.create(VSX_MODULE_PARAM_ID_INT, "interpolation");
    interpolation->set(i_interpolation);
    sequence = (vsx_module_param_sequence*)in_parameters.create(VSX_MODULE_PARAM_ID_SEQUENCE, "sequence");
    sequence->set(i_sequence);
    reverse = (vsx_module_param_int*)in_parameters.create(VSX_MODULE_PARAM_ID_INT, "reverse");
    reverse->set(i_reverse);
    reset_seq_to_default = (vsx_module_param_int*)in_parameters.create(VSX_MODULE_PARAM_ID_INT, "reset_seq_to_default");
    reset_seq_to_default->set(i_reset_seq_to_default);
    
    redeclare_out=true;
  }

  //Rebuild Outputs
  void redeclare_out_params(vsx_module_param_list& out_parameters)
  {
    result = (vsx_module_param_float4*)out_parameters.create(VSX_MODULE_PARAM_ID_FLOAT4,"result");
    result->set(0.0, 0);
    result->set(0.0, 1);
    result->set(0.0, 2);
    result->set(0.0, 3);   

    i_am_ready = true;
  }

//---CORE-FUNCTIONS-------------------------------------------------------------

  //See /vsxu/engine/include/vsx_math.h for definitions of:
    //FLOAT_EQUALS(A, B)
    //FLOAT_EXACT(A, B)
    //FLOAT_MOD(V, M)
    //FLOAT_CLAMP(V, MN, MX)
    //FLOAT_INTERPOLATE(float Y0, float Y1, float X, float X0, float X1)

  //Update Number of Inputs
  void UpdateInputs()
  {
    i_curr_inputs = inputs->get();
    if(i_prev_inputs != i_curr_inputs)
    {
      i_am_ready = false;
      i_curr_inputs = FLOAT_CLAMP(i_curr_inputs, 0, 15);
      i_prev_inputs = i_curr_inputs;
      redeclare_in = true;
    }
  }

  //Check for Reset Sequence to Default
  void ResetSequence()
  {
    i_reset_seq_to_default = reset_seq_to_default->get();
    if(i_reset_seq_to_default == 0)
    {
      i_sequence = i_seq_default;
      sequence->set(i_sequence);
      reset_seq_to_default->set(-1);
      i_reset_seq_to_default = -1;
    }
  }

  //Set-Up Data for Interpolation
  void SetupInterpolation(int i)
  {
    switch(i_wrap)
    {
      case 0: //Don't Wrap Input- Zero Ends
        i_index = FLOAT_CLAMP(index->get(), -1.0, (float)(i_prev_inputs + 1));
        i_index_x = (int)i_index;
        i_underRange = i_index < 0.0;
        i_overRange = i_index > (float)i_prev_inputs;
        i_index_x0 = (i_underRange) ? -1
                      :((i_overRange) ? i_prev_inputs
                      :  FLOAT_CLAMP(i_index_x, -1, i_prev_inputs + 1));
        i_index_x1 = (i_underRange) ? 0
                      :((i_overRange) ? i_prev_inputs + 1
                      :  FLOAT_CLAMP(i_index_x + 1, 0, i_prev_inputs + 2));
        i_value_y0[i] = (i_underRange) ? 0.0
                      :((i_overRange) ? float_x[i_prev_inputs]->get(i)
                      :  float_x[i_index_x0]->get(i));
        i_value_y1[i] = (i_underRange) ? float_x[0]->get(i)
                      :((i_overRange) ? 0.0
                      :  float_x[i_index_x1]->get(i));
      break;
      case 1: //Don't Wrap Input- Freeze Ends
        i_index = FLOAT_CLAMP(index->get(), 0.0, (float)i_prev_inputs);
        i_index_x = (int)i_index;
        i_index_x0 = FLOAT_CLAMP(i_index_x, 0, i_prev_inputs);
        i_index_x1 = FLOAT_CLAMP(i_index_x + 1, 0, i_prev_inputs);
        i_value_y0[i] = float_x[i_index_x0]->get(i);
        i_value_y1[i] = float_x[i_index_x1]->get(i);
      break;
      case 2: //Wrap Input
        i_index = FLOAT_MOD(index->get(), (float)(i_prev_inputs + 1));
        i_index_x = (int)i_index;
        i_index_x0 = i_index_x % (i_prev_inputs + 1);
        i_index_x1 = i_index_x0 + 1;
        i_value_y0[i] = float_x[i_index_x0]->get(i);
        i_value_y1[i] = (i_index_x1 > i_prev_inputs) ? float_x[0]->get(i)
                      : float_x[i_index_x1]->get(i);
      break;
    }
  }

  //Perform Linear Interpolation
  void LinearInterpolation(int i)
  {
    result->set(FLOAT_INTERPOLATE(i_value_y0[i],
                                  i_value_y1[i],
                                  i_index,
                                  (float)i_index_x0,
                                  (float)i_index_x1),
                                  i);
  }

  //Perform Interpolation Using the Sequence Graph
  void SequenceInterpolation(int i)
  {
    i_sequence = sequence->get();
    sequence->updates = 0;
    i_sequence.reset();

    //Cache Sequence Data
    for(int j = 0; j < SEQ_RESOLUTION; ++j)
    {
      i_sequence_data[j] = i_sequence.execute(1.0f / (float)(SEQ_RESOLUTION - 1));
    }
          
    i_seq_index = ((i_index - (float)i_index_x0) 
                / (float)(i_index_x1 - i_index_x0))
                * (float)SEQ_RESOLUTION;
    
    //Bugfix for No Wrap - Zero Ends on Sequence Interpolation
    if(i_wrap == 0)
    {
      i_value_y0[i] = (i_index_x == i_prev_inputs + 1) ? 0.0 : i_value_y0[i];
      i_value_y1[i] = (i_index_x == -1) ? 0.0 : i_value_y1[i];
    }
    
    //Determine Which Graph Reversal Method to Use      
    i_reverse = reverse->get();
    switch(i_reverse)
    {
      case 0: //Reverse Off
        result->set(FLOAT_INTERPOLATE(i_value_y0[i],
                                      i_value_y1[i],
                               (float)i_index_x0 + i_sequence_data[i_seq_index],
                               (float)i_index_x0,
                               (float)i_index_x1),
                                      i);
      break;
      case 1: //Reverse On
        result->set(FLOAT_INTERPOLATE(i_value_y1[i],
                                      i_value_y0[i],
                               (float)i_index_x1 - i_sequence_data[i_seq_index],
                               (float)i_index_x0,
                               (float)i_index_x1),
                                      i);
      break;
      case 2: //Autoreverse - Normal
        result->set(FLOAT_INTERPOLATE((i_value_y0[i] < i_value_y1[i]) ? i_value_y0[i] : i_value_y1[i],
                                      (i_value_y0[i] < i_value_y1[i]) ? i_value_y1[i] : i_value_y0[i],
                                      (i_value_y0[i] < i_value_y1[i]) 
                              ? (float)i_index_x0 + i_sequence_data[i_seq_index]
                              : (float)i_index_x1 - i_sequence_data[i_seq_index],
                                (float)i_index_x0,
                                (float)i_index_x1),
                                       i);
      break;
      case 3: //Autoreverse - Inverted
        result->set(FLOAT_INTERPOLATE((i_value_y0[i] < i_value_y1[i]) ? i_value_y1[i] : i_value_y0[i],
                                      (i_value_y0[i] < i_value_y1[i]) ? i_value_y0[i] : i_value_y1[i],
                                      (i_value_y0[i] < i_value_y1[i])
                              ? (float)i_index_x1 - i_sequence_data[i_seq_index]
                              : (float)i_index_x0 + i_sequence_data[i_seq_index],
                                (float)i_index_x0,
                                (float)i_index_x1),
                                       i);
      break;
    }
  }

  //Don't Interpolate
  void NoInterpolation(int i)
  {
    switch(i_wrap)
    {
      case 0: //Don't Wrap Index - Zero Ends
        i_index = FLOAT_CLAMP(index->get() + 0.5, -0.5, (float)(i_prev_inputs) + 1.5);
        i_index_x0 = FLOAT_CLAMP((int)i_index, 0, i_prev_inputs);
        i_value_y0[i] = ((i_index < -0.0) || (i_index > (float)(i_prev_inputs + 1)) ? 0.0
                      : float_x[i_index_x0]->get(i));
      break;
      case 1: //Don't Wrap Index - Freeze Ends
        i_index = FLOAT_CLAMP(index->get() + 0.5, 0.5, (float)(i_prev_inputs) + 0.5);
        i_index_x0 = FLOAT_CLAMP((int)i_index, 0, i_prev_inputs);
        i_value_y0[i] = float_x[i_index_x0]->get(i);
      break;
      case 2: //Wrap Index
        i_index = FLOAT_MOD(index->get(), (float)(i_prev_inputs + 1));
        i_index_x0 = (i_index > (float)i_prev_inputs)
                      ? ((i_index > ((float)i_prev_inputs + 0.5)) ? 0 : i_prev_inputs)
                      : (int)(i_index + 0.5);
        i_value_y0[i] = float_x[i_index_x0]->get(i);
      break;
    }
    result->set(i_value_y0[i], i);
  }

//---MAIN-PROGRAM-LOOP---------------------------------------------------------

  void run()
  {
    UpdateInputs();
    ResetSequence();

    if(i_am_ready)
    {
      i_wrap = wrap->get();
      i_interpolation = interpolation->get();
      for(int i = 0; i < 4; i++)
      {
        switch(i_interpolation)
        {
          case 0: NoInterpolation(i); break;
          case 1:
            SetupInterpolation(i);
            LinearInterpolation(i);
          break;
          case 2:
            SetupInterpolation(i);
            SequenceInterpolation(i);
          break;
        }
      }
    }
  }
};