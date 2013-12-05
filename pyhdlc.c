#include <Python.h>
#include "hdlc.h"
#include <stdio.h>

static PyObject* hdlc_pack(PyObject* self, PyObject*args) {
	int srclen;
	int dstlen;
	const char*src = NULL;
	char *dst = NULL;
	PyObject*result = NULL;

	//TODO: getbinary stream from args
	if (!PyArg_ParseTuple(args, "s#", &src, &srclen)) {
		return NULL;
	}

	//max lengthaccording to hdlc protocol
	dstlen = srclen * 2 + 3;
	dst = (char*) PyMem_Malloc(dstlen);
	if (dst == NULL) {
		return PyErr_NoMemory();
	}

	dstlen = hdlc_send_buffer(dst, src, srclen);
	result = Py_BuildValue("s#", dst, dstlen);

	PyMem_Free(dst);

	return result;
}

static PyObject* hdlc_unpack(PyObject* self, PyObject*args) {
	int srclen;
	int dstlen;
	int retlen;
	const char*src = NULL;
	const char*src_head = NULL;
	char *dst = NULL;
	HDLC hdlc;
	PyObject*result = NULL;

	//TODO: getbinary stream from args
	if (!PyArg_ParseTuple(args, "s#", &src, &srclen)) {
		return NULL;
	}

	//max lengthaccording to hdlc protocol
	dstlen = srclen;
	dst = (char*) PyMem_Malloc(dstlen);
	if (dst == NULL) {
		return PyErr_NoMemory();
	}

	hdlc_init_recv(&hdlc, dst, dstlen);

	retlen = 0;
	src_head = src;
	while (srclen-- > 0 && retlen == 0) {
		retlen = hdlc_recv_char(&hdlc, *src++);
	}

	//return atuple(data, readlen)
	result = Py_BuildValue("s#,i", dst, retlen, (int) (src - src_head));

	PyMem_Free(dst);

	return result;
}

static PyMethodDef hdlcMethods[] =
		{ { "pack", hdlc_pack, METH_VARARGS, "pack data queue withHDLC." },
				{ "unpack", hdlc_unpack, METH_VARARGS,
						"unpackdata queue with HDLC. Return a tuple containing unpacked data and length ofread data.(data, len)" },
				{ NULL, NULL } };

PyMODINIT_FUNC initpyhdlc() {
	Py_InitModule("pyhdlc", hdlcMethods);
}

