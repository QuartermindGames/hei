/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include "model_private.h"
#include "filesystem_private.h"

/* Support for Shadow Man's old and new mesh formats */

/* new formats */

static PLModel* LoadEMsh(PLFile* fp) {
  ReportError(PL_RESULT_UNSUPPORTED, "EMsh is not supported");
  return NULL;
}

/* old formats */

#define VERSION_WEI 3

typedef struct WEIHeader {
  char identity[4];    /* last byte is the version */
} WEIHeader;

static PLModel* LoadMESH(PLFile* fp) {
  bool status;
  plReadInt32(fp, false, &status);
  if (!status) {
    return NULL;
  }

  char list_identifier[4];
  if (plReadFile(fp, list_identifier, sizeof(char), 4) != 4) {
    return NULL;
  }

  if (strncmp(list_identifier, "OLST", 4) != 0) {
    ReportError(PL_RESULT_FILETYPE, "invalid identifier, expected OLST but found %s", list_identifier);
    return NULL;
  }

  char filename[64];
  strncpy(filename, plGetFileName(plGetFilePath(fp)), sizeof(filename));
  filename[strlen(filename) - 3] = '\0';

  return NULL;
}

/* */

PLModel* plLoadMSHModel(const char* path) {
  PLFile* fp = plOpenFile(path, false);
  if (fp == NULL) {
    return NULL;
  }

  char identifier[4];
  if (plReadFile(fp, identifier, sizeof(char), 4) != 4) {
    plCloseFile(fp);
    return NULL;
  }

  PLModel* model_ptr = NULL;
  if (strncmp(identifier, "EMsh", 4) == 0) {
    model_ptr = LoadEMsh(fp);
  } else if (strncmp(identifier, "MESH", 4) == 0) {
    model_ptr = LoadMESH(fp);
  } else {
    ReportError(PL_RESULT_FILETYPE, "unrecognised identifier");
  }

  plCloseFile(fp);

  return model_ptr;
}
