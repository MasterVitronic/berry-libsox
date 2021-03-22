/*

  be-libsox.c - Berry bindings to libsox

  Copyright (c) 2021, Díaz Devera Víctor <mastervitronic@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/***
 * Berry bindings to libsox
 *
 * This documentation is partial, and doesn't cover all functionality yet.
 * @module libsox
 * @author Díaz Devera Víctor (Máster Vitronic) <mastervitronic@gmail.com>
 */


#define BERRY_MODULE
#include <berry/berry.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include <sox.h>

/***
 * libSoX library version.
 * @function version
 * @treturn string version string "major.minor.revision"
 * @see sox_version
 */
static int m_sox_version(bvm *vm) {
	const char * version = sox_version();
	be_pushstring(vm,  version);
	be_return(vm);
}


/***
 * Initialize effects library.
 * @function sox_init
 * @treturn boolean true if successful.
 * @see sox_init
 */
static int m_sox_init(bvm *vm) {
	if (sox_init() != SOX_SUCCESS) {
		be_pushbool(vm, 0);			
	} else {
		be_pushbool(vm, 1);
	}
	be_return(vm);
}


/***
 * Close effects library and unload format handler plugins.
 * @function sox_quit
 * @treturn boolean true if successful.
 * @see sox_quit
 */
static int m_sox_quit(bvm *vm) {
	if (sox_quit() != SOX_SUCCESS) {
		be_pushbool(vm, 0);	
	} else {
		be_pushbool(vm, 1);
	}
	be_return(vm);
}


/***
 * Allocates size bytes in storage buffer, default is (32768).
 * @function buffer
 * @tparam integer size  size of buffer
 * @treturn userdata Returned buffer handle .
 */
static int m_buffer(bvm *vm) {
	size_t buf_sz;
	if ( be_isint(vm, 1) ){
		buf_sz = be_toint(vm, 1);
	} else {
		buf_sz = 32768;
	}
	sox_sample_t* buffer = malloc(sizeof(sox_sample_t)*buf_sz);
	be_pushcomptr(vm, buffer);
	be_return(vm);
}


/***
 * Deallocates the space previously allocated to buffer.
 * @function free_buffer
 * @tparam userdata buf the buffer handle
 */
static int m_free_buffer(bvm *vm) {
	sox_sample_t* buf = be_tocomptr(vm, 1);
	free(buf);
	be_return_nil(vm);
	be_return(vm);
}


/***
 * Converts SoX native sample to a 32-bit float.
 * @function sample_to_float32
 * @tparam userdata buf Input sample to be converted.
 * @tparam integer  index index in the buffer array
 * @tparam integer  clips Number to increment if input sample is too large.
 */
static int m_sample_to_float32(bvm *vm) {
	sox_sample_t* buf = be_tocomptr(vm, 1);
	int index         = be_toint(vm, 2);
	size_t clips      = be_toint(vm, 3);
	SOX_SAMPLE_LOCALS;
	float sample = SOX_SAMPLE_TO_FLOAT_32BIT(buf[index],clips);
	be_pushreal(vm, sample);

	be_return(vm);
}


/***
 * Converts SoX native sample to a 64-bit float.
 * @function sample_to_float64
 * @tparam userdata buf Input sample to be converted.
 * @tparam integer  index index in the buffer array
 */
static int m_sample_to_float64(bvm *vm) {
	sox_sample_t* buf = be_tocomptr(vm, 1);
	int index         = be_toint(vm, 2);
	SOX_SAMPLE_LOCALS;
	double sample = SOX_SAMPLE_TO_FLOAT_64BIT(buf[index],0);
	be_pushreal(vm, sample);

	be_return(vm);
}


/***
 * Return audio levels of the current buffer.
 * @function get_levels
 * @tparam userdata buf Input sample to be analize.
 * @tparam integer block_size the size of blocks to be read.
 * @treturn number level of right channel.
 * @treturn number level of left channel.
 */
static int m_get_levels(bvm *vm) {
	sox_sample_t* buf = be_tocomptr(vm, 1);
	size_t block_size = be_toint(vm, 2);
	double left = 0, right = 0;
	size_t i; SOX_SAMPLE_LOCALS;

	for (i = 0; i < block_size; ++i) {
		double sample = SOX_SAMPLE_TO_FLOAT_64BIT(buf[i],0);
		if (i & 1) {
			right = fmax(right, fabs(sample));
		} else {
			left = fmax(left, fabs(sample));
		}
	}

	be_newobject(vm, "list");

	be_pushint(vm, ((1 - right) * 35 + .5));
	be_data_push(vm, -2);
	be_pop(vm, 1);

	be_pushint(vm, ((1 - left) * 35 + .5));
	be_data_push(vm, -2);
	be_pop(vm, 1);

	be_pop(vm, 1);
	be_return(vm);
}


static void map_insert(bvm *vm, const char *key, int value){
    be_pushstring(vm, key);
    be_pushint(vm, value);
    be_data_insert(vm, -3);
    be_pop(vm, 2);
}


/***
 * Return information already known about audio stream, or NULL if none.
 * @function signal
 * @tparam userdata ft the current handler
 */
static int m_signal(bvm *vm) {
	sox_format_t* ft = be_tocomptr(vm, 1);

	/*signalinfo map*/
	be_newobject(vm, "map");

	map_insert(vm, "channels", ft->signal.channels);
	map_insert(vm, "length", ft->signal.length);
	//be_pushstring(vm, "mult");
		//be_pushreal(vm, ft->signal.mult); /*@FIXE type double */
		//be_data_insert(vm, -3);
	//be_pop(vm, 2);
	map_insert(vm, "precision", ft->signal.precision);
	map_insert(vm, "rate", ft->signal.rate);

	be_pop(vm, 1);
	be_return(vm);
}


/***
 * Return information already known about sample encoding
 * @function encoding
 * @tparam userdata ft the current handler
 */
static int m_encoding(bvm *vm) {
	sox_format_t* ft = be_tocomptr(vm, 1);

	/*encoding table*/
	be_newobject(vm, "map");

	map_insert(vm, "bits_per_sample", ft->encoding.bits_per_sample);
	map_insert(vm, "compression", ft->encoding.compression);
	map_insert(vm, "encoding", ft->encoding.encoding);
	be_pushstring(vm, "opposite_endian");
		be_pushbool(vm, ft->encoding.opposite_endian);
		be_data_insert(vm, -3);
	be_pop(vm, 2);

	be_pop(vm, 1);
	be_return(vm);
}


/***
 * Initializes an effects chain. 
 * @function sox_create_effects_chain
 * @tparam userdata in_enc Input encoding.
 * @tparam userdata out_enc Output encoding.
 * @treturn userdata handle Returned must be closed with sox:delete_effects_chain().
 * @see sox_create_effects_chain
 */
static int m_sox_create_effects_chain(bvm *vm) {
	sox_format_t* in_enc  = be_tocomptr(vm, 1);
	sox_format_t* out_enc = be_tocomptr(vm, 2);
	sox_effects_chain_t * handle = sox_create_effects_chain(
		&in_enc->encoding, &out_enc->encoding
	);

	if ( handle == NULL ){
		be_return_nil(vm);
	} else {
		be_pushcomptr(vm, handle);
	}

	be_return(vm);
}


/***
 * Creates an effect using the given handler.
 * @function sox_create_effect
 * @tparam userdata eh Handler to use for effect.
 * @see sox_create_effect
 */
static int m_sox_create_effect(bvm *vm) {
	sox_effect_handler_t* eh  = be_tocomptr(vm, 1);
	sox_effect_t* effect = sox_create_effect(eh);

	if ( effect == NULL ){
		be_return_nil(vm);
	} else {
		be_pushcomptr(vm, effect);
	}

	be_return(vm);
}


/***
 * Finds the effect handler with the given name.
 * @function sox_find_effect
 * @tparam string name Name of effect to find.
 * @see sox_find_effect
 */
static int m_sox_find_effect(bvm *vm) {
	char const* name  = be_tostring(vm, 1);
	const sox_effect_handler_t* effect = sox_find_effect(name);

	if ( effect == NULL ){
		be_return_nil(vm);
	} else {
		be_pushcomptr(vm, (void*)effect);
	}

	be_return(vm);
}


/***
 * Applies the command-line options to the effect.
 * @function sox_effect_options
 * @tparam userdata effp Effect pointer on which to set options.
 * @tparam integer argc Number of arguments in argv.
 * @tparam userdata|string argv Array of command-line options.
 * @see sox_effect_options
 */
static int m_sox_effect_options(bvm *vm) {
	sox_effect_t *  effp  = be_tocomptr(vm, 1);
	int argc              = be_toint(vm, 2);
	char * args[10];
	if ( be_iscomptr(vm, 3) ){
		sox_effect_t *  argv  = be_tocomptr(vm, 3);
		args[0] = (char *)argv;
	} else if ( be_isstring(vm, 3) ) {
		char const  * argv  = be_tostring(vm, 3);
		args[0] = (char *)argv;
	}

	int rc = sox_effect_options(effp, argc, args);

	be_pushint(vm, rc);

	be_return(vm);
}


/***
 * Adds an effect to the effects chain.
 * @function sox_add_effect
 * @tparam userdata chain Effects chain to which effect should be added .
 * @tparam userdata effp Effect to be added.
 * @tparam userdata in Input format.
 * @tparam userdata out Output format.
 * @treturn boolean returns true if successful.
 * @see sox_add_effect
 */
static int m_sox_add_effect(bvm *vm) {
	sox_effects_chain_t * chain  = be_tocomptr(vm, 1);
	sox_effect_t *        effp   = be_tocomptr(vm, 2);
	sox_format_t*         in     = be_tocomptr(vm, 3);
	sox_format_t*         out    = be_tocomptr(vm, 4);
	int success = sox_add_effect(
		chain, effp, &in->signal, &out->signal
	);

	if (success != SOX_SUCCESS) {
		be_pushbool(vm, 0);
	} else {
		be_pushbool(vm, 1);
	}

	be_return(vm);
}


/***
 * Runs the effects chain.
 * @function sox_flow_effects
 * @tparam userdata chain Effects chain to run.
 * @see sox_flow_effects
 * @treturn boolean returns true if successful.
 * @todo WIP
 */
 //* @tparam Callback for monitoring flow progress.
 //* @tparam Data to pass into callback.
static int m_sox_flow_effects(bvm *vm) {
	sox_effects_chain_t * chain  = be_tocomptr(vm, 1);
	//sox_flow_effects_callback callback   = be_tocomptr(vm, 3);
	//void * 	client_data    = be_tocomptr(vm, 4);
	//int success = sox_flow_effects(chain, callback, client_data);
	int success = sox_flow_effects(chain, NULL, NULL);
	
	if (success != SOX_SUCCESS) {
		be_pushbool(vm, 0);
	} else {
		be_pushbool(vm, 1);
	}

	be_return(vm);
}


/***
 * Closes an effects chain.
 * @function sox_delete_effects_chain
 * @tparam userdata ecp Effects chain pointer.
 * @see sox_delete_effects_chain
 */
static int l_sox_delete_effects_chain(bvm *vm) {
	sox_effects_chain_t * ecp  = be_tocomptr(vm, 1);
	sox_delete_effects_chain(ecp);

	be_return(vm);
}


/***
 * Opens a decoding session for a file.
 * @function sox_open_read
 * @tparam string path Path to file to be opened (required).
 * @treturn userdata handle Returned handle must be closed with sox:close(handle).
 * @see sox_open_read
 */
static int m_sox_open_read(bvm *vm) {
	char const  * path  = be_tostring(vm, 1);
	sox_format_t* input = sox_open_read(path, NULL, NULL, NULL);

	if ( input == NULL ){
		be_return_nil(vm);
	} else {
		be_pushcomptr(vm, input);
	}

	be_return(vm);
}


/***
 * Reads samples from a decoding session into a sample buffer.
 * @function sox_read
 * @tparam userdata ft Format pointer.
 * @tparam userdata buf Buffer from which to read samples.
 * @tparam integer len Number of samples available in buf.
 * @treturn integer Number of samples decoded, or 0 for EOF.
 * @see sox_read
 */
static int m_sox_read(bvm *vm) {
	sox_format_t* ft  = be_tocomptr(vm, 1);
	sox_sample_t* buf = be_tocomptr(vm, 2);
	size_t len  	  = be_toint(vm, 3);
	size_t sz 	  = sox_read(ft, buf, len);
	be_pushint(vm, sz);
	be_return(vm);
}


/***
 * Opens an encoding session for a file. Returned handle must be closed with sox:close().
 * @function sox_open_write
 * @tparam string path Path to file to be written (required).
 * @tparam array signal Information about desired audio stream (required).
 * @tparam string filetype Previously-determined file type, or NULL to auto-detect.
 * @treturn userdata|nil handle The new session handle, or nil on failure..
 * @see sox_open_write
 */
static int m_sox_open_write(bvm *vm) {
	char const * path     = be_tostring(vm, 1);
	char const * filetype = be_tostring(vm, 3);
	sox_signalinfo_t signal;

	if ( be_iscomptr(vm, 2) ){
		sox_format_t* ft =  be_tocomptr(vm, 2);
		signal = ft->signal;	
	} else {
		sox_signalinfo_t signal = {};
		be_raise(vm, "type_error", "Not suport yet");
		/*@TODO*/
		//signal.rate = be_getmember(vm, 1, "rate");
		//signal.channels = be_getmember(vm, 2, "channels");
		//signal.precision = be_getmember(vm, 3, "precision");
	}

	sox_format_t* output = sox_open_write(
		path, &signal, NULL, filetype, NULL, NULL
	);

	if ( output == NULL ) {
		be_return_nil(vm);
	} else {
		be_pushcomptr(vm, output);
	}

	be_return(vm);
	return 0;
}


/***
 * Writes samples to an encoding session from a sample buffer.
 * @function sox_write
 * @tparam userdata ft Format pointer.
 * @tparam userdata buf Buffer from which to read samples.
 * @tparam integer len Number of samples available in buf.
 * @treturn integer Number of samples encoded..
 * @see sox_write
 */
static int m_sox_write( bvm *vm ) {
	if (!be_iscomptr(vm, 1) || !be_iscomptr(vm, 2)) {
		be_raise(vm, "type_error", "Invalid argument");
	}
	sox_format_t* ft  = be_tocomptr(vm, 1);
	sox_sample_t* buf = be_tocomptr(vm, 2);
	size_t len        = be_toint(vm, 3);
	size_t sz	  = sox_write(ft, buf, len);

	be_pushint(vm, sz);
	be_return(vm);
}


/***
 * Sets the location at which next samples will be decoded.
 * @function sox_seek
 * @tparam userdata ft Format pointer.
 * @tparam integer offset Sample offset at which to position reader.
 * @treturn boolean returns true if successful.
 * @see sox_seek
 */
static int m_sox_seek(bvm *vm) {
	sox_format_t* ft    = be_tocomptr(vm, 1);
	sox_uint64_t offset = be_toint(vm, 2);
	if (sox_seek(ft, offset, SOX_SEEK_SET) != SOX_SUCCESS) {
		be_pushbool(vm, 0);
	} else {
		be_pushbool(vm, 1);
	}
	be_return(vm);
}


/***
 * Closes an encoding or decoding session.
 * @function sox_close
 * @treturn boolean returns true if successful.
 * @see sox_close
 */
static int m_sox_close(bvm *vm) {
	if (!be_iscomptr(vm, 1)) {
		be_raise(vm, "type_error", "Invalid argument");
	}
	sox_format_t* ft = be_tocomptr(vm, 1);
	if (sox_close(ft) != SOX_SUCCESS) {
		be_pushbool(vm, 0);
	} else {
		be_pushbool(vm, 1);
	}
	be_return(vm);
}


static void bind_member(bvm *vm, const char *attr, bntvfunc f) {
    be_pushntvfunction(vm, f);
    be_setmember(vm, -2, attr);
    be_pop(vm, 1);
}


int berry_export(bvm *vm) {
    be_newmodule(vm);
    be_setname(vm, -1, "libsox");
    bind_member(vm, "version", m_sox_version);
    bind_member(vm, "init", m_sox_init);
    bind_member(vm, "quit", m_sox_quit);
    bind_member(vm, "buffer", m_buffer);
    bind_member(vm, "free_buffer", m_free_buffer);
    bind_member(vm, "sample_to_float32", m_sample_to_float32);
    bind_member(vm, "sample_to_float64", m_sample_to_float64);
    bind_member(vm, "get_levels", m_get_levels);
    bind_member(vm, "signal", m_signal);
    bind_member(vm, "encoding", m_encoding);
    bind_member(vm, "create_effects_chain", m_sox_create_effects_chain);
    bind_member(vm, "create_effect", m_sox_create_effect);
    bind_member(vm, "find_effect", m_sox_find_effect);
    bind_member(vm, "effect_options", m_sox_effect_options);
    bind_member(vm, "add_effect", m_sox_add_effect);
    bind_member(vm, "flow_effects", m_sox_flow_effects);
    bind_member(vm, "open_read", m_sox_open_read);
    bind_member(vm, "read", m_sox_read);
    bind_member(vm, "open_write", m_sox_open_write);
    bind_member(vm, "write", m_sox_write);
    bind_member(vm, "seek", m_sox_seek);
    bind_member(vm, "close", m_sox_close);
    be_return(vm);
}
