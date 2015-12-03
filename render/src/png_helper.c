#include <stdio.h>

#include "bcm_host.h"
#include "ilclient.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#define TIMEOUT_MS 2000

static ILCLIENT_T* client;

static COMPONENT_T* encoder;
static OMX_HANDLETYPE encoder_handle;
static int encoder_inport;
static int encoder_outport;

static int image_width;
static int image_height;

static int ready = 0;
static int size_ready = 0;

#define min(a, b) (a < b ? a : b)

static int align(int length) {
	return (((length - 1) >> 4) + 1) << 4;
}

static int align_image(char* image, int width, int height, char** buffer) {
	int width_aligned = align(width);
	int height_aligned = align(height);

	char* output = (char*) calloc(width_aligned * height_aligned * 3, sizeof(char));

	int i;
	for(i = 0; i < height; i++) {
		int destination = i * width_aligned * 3;
		int source = i * width * 3;
		memcpy((void*) (output + destination), (void*) (image + source), width * 3);
	}
	
	*buffer = output;

	return width_aligned * height_aligned * 3;
}

int png_init() {
	if(ready)
		return 1;
	
	bcm_host_init();

	if(!(client = ilclient_init())) {
		fprintf(stderr, "Unable to initialize ILClient\n");
		return 0;
	}

	if(OMX_Init() != OMX_ErrorNone) {
		ilclient_destroy(client);
		fprintf(stderr, "Unable to initialize OpenMax\n");
		return 0;
	}



	if(ilclient_create_component(client, &encoder, "image_encode", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_ENABLE_OUTPUT_BUFFERS)) {
		fprintf(stderr, "Unable to create encoder component\n");
		return 0;
	}

	encoder_handle = ilclient_get_handle(encoder);


	OMX_PORT_PARAM_TYPE port;
	port.nSize = sizeof(OMX_PORT_PARAM_TYPE);
	port.nVersion.nVersion = OMX_VERSION;

	if(OMX_GetParameter(encoder_handle, OMX_IndexParamImageInit, &port) != OMX_ErrorNone) {
		fprintf(stderr, "Unable to get encoder ports\n");
		return 0;
	}

	if(port.nPorts != 2) {
		fprintf(stderr, "No ports on JPEG encoder\n");
		return 0;
	}

	encoder_inport = port.nStartPortNumber;
	encoder_outport = port.nStartPortNumber + 1;



	ready = 1;

	return 1;
}

int png_set_size(int width, int height) {
	if(ilclient_change_component_state(encoder, OMX_StateLoaded)) {
		fprintf(stderr, "Unable to set encoder state to loaded\n");
		return 0;
	}

	OMX_PARAM_PORTDEFINITIONTYPE encoder_portdef;
	encoder_portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	encoder_portdef.nVersion.nVersion = OMX_VERSION;
	encoder_portdef.nPortIndex = encoder_inport;

	if(OMX_GetParameter(encoder_handle, OMX_IndexParamPortDefinition, &encoder_portdef) != OMX_ErrorNone) {
		fprintf(stderr, "Unable to get encoder port definition\n");
		return 0;
	}

	encoder_portdef.format.image.nFrameWidth = width;
	encoder_portdef.format.image.nFrameHeight = height;
	encoder_portdef.format.image.nStride = align(width) * 3;
	encoder_portdef.format.image.nSliceHeight = align(height);
	encoder_portdef.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
	encoder_portdef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	encoder_portdef.nBufferSize = align(width) * align(height) * 3;

	int err;
	if((err = OMX_SetParameter(encoder_handle, OMX_IndexParamPortDefinition, &encoder_portdef)) != OMX_ErrorNone) {
		fprintf(stderr, "Unable to set encoder port definition\n");
		return 0;
	}

	OMX_IMAGE_PARAM_PORTFORMATTYPE encoder_format;
	memset(&encoder_format, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
	encoder_format.nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
	encoder_format.nVersion.nVersion = OMX_VERSION;
	encoder_format.nPortIndex = encoder_outport;
	encoder_format.eCompressionFormat = OMX_IMAGE_CodingPNG;
	encoder_format.eColorFormat = OMX_COLOR_FormatUnused;

	if(OMX_SetParameter(encoder_handle, OMX_IndexParamImagePortFormat, &encoder_format) != OMX_ErrorNone) {
		fprintf(stderr, "Unable to set encoder format\n");
		return 0;
	}

	if(OMX_GetParameter(encoder_handle, OMX_IndexParamImagePortFormat, &encoder_format) != OMX_ErrorNone) {
		fprintf(stderr, "Unable to get encoder format\n");
		return 0;
	}

	if(ilclient_change_component_state(encoder, OMX_StateIdle)) {
		fprintf(stderr, "Unable to set encoder state to idle\n");
		return 0;
	}

	if(OMX_SendCommand(encoder_handle, OMX_CommandPortEnable, encoder_inport, NULL) != OMX_ErrorNone) {
		fprintf(stderr, "Unable to enable encoder input port\n");
		return 0;
	}

	if(ilclient_enable_port_buffers(encoder, encoder_outport, NULL, NULL, NULL)) {
		fprintf(stderr, "Unable to enable encoder output port buffer\n");
		return 0;
	}

	image_width = width;
	image_height = height;

	size_ready = 1;

	return 1;
}

int png_encode(EGLImageKHR eglImage, char** output_image) {
	if(!(ready && size_ready))
		return -1;

	OMX_BUFFERHEADERTYPE* buffer;

	int err;
	if((err = OMX_UseEGLImage(encoder_handle, &buffer, encoder_inport, NULL, eglImage)) != OMX_ErrorNone) {
		fprintf(stderr, "Unable to set EGLImage\n");
		return -1;
	}

	if(ilclient_change_component_state(encoder, OMX_StateExecuting)) {
		fprintf(stderr, "Unable to set encoder state to executing\n");
		return -1;
	}

	if(OMX_EmptyThisBuffer(encoder_handle, buffer) != OMX_ErrorNone) {
		fprintf(stderr, "Error emptying buffer\n");
		return -1;
	}

	char* output = NULL;
	int read_out = 0;

	while(1) {
		buffer = ilclient_get_output_buffer(encoder, encoder_outport, 1);

		if(buffer) {
			output = realloc(output, read_out + buffer->nFilledLen);
			memcpy(output + read_out, buffer->pBuffer, buffer->nFilledLen);

			read_out += buffer->nFilledLen;

			if(buffer->nFlags & OMX_BUFFERFLAG_EOS)
				break;

			if(OMX_FillThisBuffer(encoder_handle, buffer) != OMX_ErrorNone)
				fprintf(stderr, "Error filling buffer\n");
		}
	}

	if(ilclient_change_component_state(encoder, OMX_StateIdle)) {
		fprintf(stderr, "Unable to set encoder state to idle\n");
		return -1;
	}

	*output_image = output;

	return read_out;
}

int png_deinit() {
	if(!ready)
		return 1;

	COMPONENT_T* components[2];
	components[0] = encoder;
	components[1] = NULL;
	
	ilclient_cleanup_components(components);
	ilclient_destroy(client);
	OMX_Deinit();
	bcm_host_deinit();

	ready = 0;

	return 1;
}
