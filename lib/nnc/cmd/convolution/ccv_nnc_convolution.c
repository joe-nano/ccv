#include <ccv.h>
#include <nnc/ccv_nnc.h>
#include <nnc/ccv_nnc_easy.h>
#include <nnc/ccv_nnc_internal.h>

static int _ccv_nnc_conv_forw_bitmask(const uint64_t input_bitmask, const uint64_t output_bitmask)
{
	if ((input_bitmask & 7u) == ((1u << 0) | (1u << 1) | (1u << 2)) && output_bitmask == 1u)
		return 1;
	return 0;
}

static int _ccv_nnc_conv_back_bitmask(const uint64_t input_bitmask, const uint64_t output_bitmask)
{
	// Output the propagated error, gradient w.r.t. w and bias.
	if ((input_bitmask & 15u) == ((1u << 0) | (1u << 1) | (1u << 2) | (0 << 3)) && output_bitmask == ((1u << 0) | (1u << 1) | (1u << 2)))
		return 1;
	// Don't propagate error, only gradient w.r.t. w and bias.
	if ((input_bitmask & 15u) == ((1u << 0) | (1u << 1) | (0 << 2) | (0 << 3)) && output_bitmask == ((0 << 0) | (1u << 1) | (1u << 2)))
		return 1;
	return 0;
}

static void _ccv_nnc_conv_tensor_auto_forw(const ccv_nnc_cmd_param_t cmd, const ccv_nnc_tensor_param_t* inputs, const int input_size, const ccv_nnc_hint_t hint, ccv_nnc_tensor_param_t* outputs, const int output_size)
{
	assert(output_size == 1);
	outputs[0].type = inputs[0].type;
	outputs[0].format = inputs[0].format;
	// Get the channel output from the weight matrix.
	int count = ccv_nnc_tensor_get_n(inputs[1]);
	assert(count == cmd.convolution.count);
	assert(count == inputs[2].dim[0]); // from the bias matrix.
	ccv_nnc_tensor_set_c(outputs, count);
	ccv_nnc_hint_tensor_forward(cmd, inputs[0], hint, outputs);
}

REGISTER_COMMAND(CCV_NNC_CONVOLUTION_FORWARD)(ccv_nnc_cmd_registry_t* registry)
	FIND_BACKEND(ccv_nnc_conv_cpu_ref.c, ccv_nnc_conv_cpu_opt.c, ccv_nnc_conv_gpu_cudnn.cu)
{
	registry->bitmask = _ccv_nnc_conv_forw_bitmask;
	registry->tensor_auto = _ccv_nnc_conv_tensor_auto_forw;
}

REGISTER_COMMAND(CCV_NNC_CONVOLUTION_BACKWARD)(ccv_nnc_cmd_registry_t* registry)
	FIND_BACKEND(ccv_nnc_conv_cpu_ref.c, ccv_nnc_conv_cpu_opt.c, ccv_nnc_conv_gpu_cudnn.cu)
{
	registry->bitmask = _ccv_nnc_conv_back_bitmask;
	registry->tensor_auto = ccv_nnc_hint_tensor_auto_backward_from_inputs;
}