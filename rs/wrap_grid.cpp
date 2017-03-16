#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

#include "js/jsdbgapi.h"
#include "js/jscntxt.h"

/** \file
Grid commands
STATUS
  RUNNING program-name
  IDLE
INFO
  The script's most recent status text
SEND message length\ncontents
  Send a message to the script
RUN
PROVISION script_package
DEBUG server:port
  Attach to a debug server


Grid config
 receives files?
 sends files?
 gateways
 allowed_programs

Node table
 address
 class (ad
*/

class Grid
{
 private:
 TStr status;

 public:
 struct Node
 {
   char address[128];
   char
 }
 struct Message
 {
   size_t length;
   const char* data;
 };
 void setStatus(const char* s);


};
