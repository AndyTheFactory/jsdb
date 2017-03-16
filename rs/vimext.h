#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*                                                              */
/*  VIMEXT.H Includes Lotus specific definitions.               */
/*                                                              */
/****************************************************************/

#ifndef __VIMEXT_H
#define __VIMEXT_H

/*  Lotus extension flags. */

#define VIM_NO_FLAGS                            0x00000000L

/*  VIM DLL name. */

#ifdef MAC
#define VIMDLL                                  "VIM"
#else
#ifdef __WIN32__
#define VIMDLL                                  "VIM32.DLL"
#else
#define VIMDLL                                  "VIM.DLL"
#endif
#endif

/*  Entry points by ordinal value. */

#define VIMORD_INITIALIZE                       1
#define VIMORD_QUERYCAPABILITY                  2
#define VIMORD_GETDEFAULTSESSIONINFO            3
#define VIMORD_OPENSESSION                      4
#define VIMORD_GETENTITYNAME                    5
#define VIMORD_CLOSESESSION                     6
#define VIMORD_TERMINATE                        7
#define VIMORD_STATUSTEXT                       8
#define VIMORD_CREATEMESSAGE                    9
#define VIMORD_CREATEDERIVEDMAILMESSAGE         10
#define VIMORD_SETMESSAGEHEADER                 11
#define VIMORD_SETMESSAGERECIPIENT              12
#define VIMORD_SETMESSAGEITEM                   13
#define VIMORD_SENDMESSAGE                      14
#define VIMORD_CLOSEMESSAGE                     15
#define VIMORD_OPENMESSAGECONTAINER             16
#define VIMORD_ENUMERATEMESSAGES                17
#define VIMORD_ENUMERATEMSGATTRVALUES           18
#define VIMORD_REMOVEMESSAGE                    19
#define VIMORD_QUERYNEWMESSAGES                 20
#define VIMORD_QUERYUNREADMAILCOUNT             21
#define VIMORD_EXTRACTMESSAGE                   22
#define VIMORD_OPENEXTRACTEDMESSAGE             23
#define VIMORD_CLOSEMESSAGECONTAINER            24
#define VIMORD_OPENMESSAGE                      25
#define VIMORD_MARKMESSAGEASREAD                26
#define VIMORD_GETMESSAGEHEADER                 27
#define VIMORD_ENUMERATEMESSAGEHDRATTRS         28  /* Note abbreviation */
#define VIMORD_ENUMERATEMESSAGEITEMS            29
#define VIMORD_GETMESSAGEITEM                   30
#define VIMORD_OPENMESSAGEITEM                  31
#define VIMORD_READMESSAGEITEM                  32
#define VIMORD_CLOSEMESSAGEITEM                 33
#define VIMORD_ENUMERATEMESSAGERECIPS           34  /* Note abbreviation */
#define VIMORD_OPENNESTEDMESSAGE                35
#define VIMORD_VERIFYMESSAGESIGNATURE           36
#define VIMORD_ENUMERATEADDRESSBOOKS            37
#define VIMORD_OPENADDRESSBOOK                  38
#define VIMORD_SETCURRENTSUBTREE                39
#define VIMORD_GETCURRENTSUBTREE                40
#define VIMORD_ENUMERATEABENTRIES               41  /* Note abbreviation */
#define VIMORD_CREATEADDRESSBOOKENTRY           42
#define VIMORD_MATCHADDRESSBOOK                 43
#define VIMORD_GETABENTRYATTRIBUTES             44
#define VIMORD_SETABENTRYATTRIBUTES             45
#define VIMORD_ENUMERATEABENTRYATTRNMS          46  /* Note abbreviation */
#define VIMORD_GETABENTRYNAMEDATTRS             47  /* Note abbreviation */
#define VIMORD_SETABENTRYNAMEDATTRS             48  /* Note abbreviation */
#define VIMORD_REMOVEADDRESSBOOKENTRY           49
#define VIMORD_ADDGROUPMEMBER                   50
#define VIMORD_REMOVEGROUPMEMBER                51
#define VIMORD_ENUMERATEGROUPMEMBERS            52
#define VIMORD_CLOSEADDRESSBOOK                 53

/*  Lotus extension entry points by ordinal value. */

/*  <used>                                      1000 */
#define VIMORD_OPENGATEWAYSESSION               1001
#define VIMORD_REMOVEMESSAGECATEGORY            1002
#define VIMORD_SETMESSAGECATEGORY               1003
#define VIMORD_GETSESSIONATTRIBUTES             1004
#define VIMORD_SETSESSIONATTRIBUTES             1005
#define VIMORD_CREATEMESSAGEEXTENDED            1006
#define VIMORD_CONVERTUNIDTOREF                 1007
#define VIMORD_OPENSHAREDCONTAINER              1008

/*  Lotus extension selectors. */

#define VIMSEL_REMOTE_LOCAL                     254L
#define VIMSEL_IMPLEMENTATION_VERSION_STR       255L      // Added for version string
#define VIMSEL_CATEGORY                         256L
#define VIMSEL_LOCATION                         257L
#define VIMSEL_ALIAS                            258L
#define VIMSEL_DIALIN                           259L
#define VIMSEL_REMOTE                           260L
#define VIMSEL_FAN                              261L
#define VIMSEL_RECIP_STATUS                     262L
#define VIMSEL_SENT_TO                          263L
#define VIMSEL_SENT_CC                          264L
#define VIMSEL_SENT_BCC                         265L
#define VIMSEL_MSG_LEN                          266L
#define VIMSEL_UNDELIVERED                      267L
#define VIMSEL_ENTRY_NUMBER                     268L
#define VIMSEL_TOTAL_ENTRIES                    269L
#define VIMSEL_CD                               270L
#define VIMSEL_IMPLEMENTATION                   271L
#define VIMSEL_IMPLEMENTATION_VERSION           272L
#define VIMSEL_DIRECT_POSTOFFICE                273L
#define VIMSEL_INDIRECT_POSTOFFICE              274L
#define VIMSEL_PASSWORD                         275L
#define VIMSEL_SHARED_CONTAINER                 276L
#define VIMSEL_CP932                            277L
#define VIMSEL_CP942                            278L
#define VIMSEL_BIG5                             279L
#define VIMSEL_CP949                            280L
#define VIMSEL_GB2312                           281L
#define VIMSEL_CP866                            282L
#define VIMSEL_CP852                            283L
#define VIMSEL_CP1250                           284L
#define VIMSEL_CP1251                           285L
#define VIMSEL_CP1253                           286L
#define VIMSEL_CP1254                           287L
#define VIMSEL_CP1255                           288L
#define VIMSEL_CP1256                           289L
#define VIMSEL_UNREAD                           290L
#define VIMSEL_ENTITY_BYLASTNAME                291L
#define VIMSEL_CP851                            292L
#define VIMSEL_CP857                            293L
#define VIMSEL_CP860                            294L
#define VIMSEL_CP861                            295L
#define VIMSEL_CP862                            296L
#define VIMSEL_CP863                            297L
#define VIMSEL_CP865                            298L
#define VIMSEL_CPISO646                         299L
#define VIMSEL_CPISO88591                       300L
#define VIMSEL_CPMacScript0                     301L
#define VIMSEL_POSTOFFICE_NAME                  302L
#define VIMSEL_POSTOFFICE_PATH                  303L
/*  <reserved>                                  304L to 511L */

/*  Lotus extension symbolic names. */
#define VIM_CD                                  "VIM_CD"

/*  Lotus extension status codes. */

#define VIMSTS_NO_RECIPIENTS                    256
#define VIMSTS_RECIPIENT_NOT_FOUND              257
#define VIMSTS_RECIPIENT_NOT_UNIQUE             258
#define VIMSTS_CONTAINER_CORRUPT                259
#define VIMSTS_INVALID_MESSAGE                  260
#define VIMSTS_USER_CANCEL                      261

/*  Lotus Notes datatypes for VIMSEL_APP_DEFINED_ITEMS. */

#ifdef VIM_NOTES
#define NOTES_TYPE_TEXT             "NOTES_TYPE_TEXT"
#define NOTES_TYPE_TEXT_LIST        "NOTES_TYPE_TEXT_LIST"
#define NOTES_TYPE_TIME             "NOTES_TYPE_TIME"
#define NOTES_TYPE_COMPOSITE        "NOTES_TYPE_COMPOSITE"
#define NOTES_TYPE_NUMBER           "NOTES_TYPE_NUMBER"
#define NOTES_TYPE_NUMBER_RANGE     "NOTES_TYPE_NUMBER_RANGE"
#define NOTES_TYPE_TIME_RANGE       "NOTES_TYPE_TIME_RANGE"
#define NOTES_TYPE_FORMULA          "NOTES_TYPE_FORMULA"
#define NOTES_TYPE_EXPRESSION       "NOTES_TYPE_EXPRESSION"
#define NOTES_TYPE_USERID           "NOTES_TYPE_USERID"
#define NOTES_TYPE_COLLATION        "NOTES_TYPE_COLLATION"
#define NOTES_TYPE_OBJECT           "NOTES_TYPE_OBJECT"
#define NOTES_TYPE_NOTEREF_LIST     "NOTES_TYPE_NOTEREF_LIST"
#define NOTES_TYPE_VIEW_FORMAT      "NOTES_TYPE_VIEW_FORMAT"
#define NOTES_TYPE_ICON             "NOTES_TYPE_ICON"
#define NOTES_TYPE_NOTELINK_LIST    "NOTES_TYPE_NOTELINK_LIST"
#define NOTES_TYPE_SIGNATURE        "NOTES_TYPE_SIGNATURE"
#define NOTES_TYPE_SEAL             "NOTES_TYPE_SEAL"
#define NOTES_TYPE_SEALDATA         "NOTES_TYPE_SEALDATA"
#define NOTES_TYPE_SEAL_LIST        "NOTES_TYPE_SEAL_LIST"
#define NOTES_TYPE_HIGHLIGHTS       "NOTES_TYPE_HIGHLIGHTS"
#endif  /* VIM_NOTES */

/*  Lotus extension functions. */

vimStatus VIMAPIENTRY VIMOpenGatewaySession(    vimStringPtr psPathSpec,
                                                vimStringPtr psPostOffice,
                                                vimStringPtr psPass,
                                                vimDWord dwVersion,
                                                vimSelector selCharSet,
                                                vimSesPtr pSession);

vimStatus VIMAPIENTRY VIMRemoveMessageCategory( vimMsgContainer msgContr,
                                                vimRef msgRef,
                                                vimStringPtr psCatName);

vimStatus VIMAPIENTRY VIMSetMessageCategory(    vimMsgContainer msgContr,
                                                vimRef msgRef,
                                                vimStringPtr psCatName);

vimStatus VIMAPIENTRY VIMGetSessionAttributes(  vimSes session,
                                                vimWord attrCount,
                                                vimAttrDescPtr pAttrs );

vimStatus VIMAPIENTRY VIMSetSessionAttributes(  vimSes session,
                                                vimWord attrCount,
                                                vimAttrDescPtr pAttrs );

vimStatus VIMAPIENTRY VIMCreateMessageExtended( vimSes Session,
                                                vimMsgContainer Container,
                                                vimStringPtr psType,
                                                vimMsgPtr pMessage);

vimStatus VIMAPIENTRY VIMConvertUNIDToRef(      vimMsgContainer Container,
                                                vimStringPtr pUNIDStr, 
                                                vimRefPtr retMsgRef);

vimStatus VIMAPIENTRY VIMOpenSharedContainer(   vimSes Session,
                                                vimStringPtr psContainerName,
                                                vimMsgContainerPtr pContainer);

/*  Standard VIM function pointer datatypes. */

typedef vimStatus (VIMCALLBACK PVIMADDGROUPMEMBER)
                                (vimAddressBook addressBook,
                                vimRef entryRef,
                                vimStringPtr entryName,
                                vimDistinguishedNamePtr pDisName);

typedef vimStatus (VIMCALLBACK PVIMCLOSEADDRESSBOOK)
                                (vimAddressBook addrBook);

typedef vimStatus (VIMCALLBACK PVIMCLOSEMESSAGE)
                                (vimMsg message);

typedef vimStatus (VIMCALLBACK PVIMCLOSEMESSAGECONTAINER)
                                (vimMsgContainer msgContainer);

typedef vimStatus (VIMCALLBACK PVIMCLOSEMESSAGEITEM)
                                (vimMsgItem msgItem);

typedef vimStatus (VIMCALLBACK PVIMCLOSESESSION)
                                (vimSes session);

typedef vimStatus (VIMCALLBACK PVIMCREATEADDRESSBOOKENTRY)
                                (vimAddressBook addressBook,
                                vimSelector entryType,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrDesc);

typedef vimStatus (VIMCALLBACK PVIMCREATEDERIVEDMAILMESSAGE)
                                (vimMsg message,
                                vimSelector selMsgType,
                                vimDWord dwFlags,
                                vimMsgPtr pMessage);

typedef vimStatus (VIMCALLBACK PVIMCREATEMESSAGE)
                                (vimSes session,
                                vimStringPtr psType,
                                vimMsgPtr pMessage);

typedef vimStatus (VIMCALLBACK PVIMENUMERATEABENTRYATTRNAMES)
                                (vimAddressBook addressBook,
                                vimRef entryRef,
                                vimStringPtr entryName,
                                vimEnumRefPtr pPos,
                                vimInt skipCount,
                                vimWord buffEntrySize,
                                vimStringPtr pBuffArray,
                                vimWordPtr pCount,
                                vimBoolPtr pMore);

typedef vimStatus (VIMCALLBACK PVIMENUMERATEADDRESSBOOKENTRIES)
                                (vimAddressBook addressBook,
                                vimRef subtreeRef,
                                vimStringPtr subtreeName,
                                vimEnumRefPtr pPos,
                                vimInt skipCount,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrDesc,
                                vimWordPtr pCount,
                                vimSelector selFiltType,
                                vimDataPtr pFiltData,
                                vimBoolPtr pMore);

typedef vimStatus (VIMCALLBACK PVIMENUMERATEADDRESSBOOKS)
                                (vimSes session,
                                vimEnumRefPtr pPos,
                                vimInt skipCount,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrDesc,
                                vimWordPtr pCount,
                                vimBoolPtr pMore);

typedef vimStatus (VIMCALLBACK PVIMENUMERATEGROUPMEMBERS)
                                (vimAddressBook addressBook,
                                vimRef entryRef,
                                vimStringPtr entryName,
                                vimEnumRefPtr pPos,
                                vimInt skipCount,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrDesc,
                                vimWordPtr pCount,
                                vimBoolPtr pMore);

typedef vimStatus (VIMCALLBACK PVIMENUMERATEMESSAGES)
                                (vimMsgContainer msgContainer,
                                vimEnumRefPtr pPos,
                                vimInt skipCount,
                                vimWord attribCount,
                                vimAttrDescPtr pAttribDesc,
                                vimWordPtr pMsgCount,
                                vimSelector selFiltType,
                                vimDataPtr pFiltData,
                                vimDWord flags,
                                vimBoolPtr pMore);

typedef vimStatus (VIMCALLBACK PVIMENUMERATEMESSAGEHEADERATTRS)
                                (vimMsg msg,
                                vimSelector selAttr,
                                vimEnumRefPtr pPos,
                                vimInt skipCount,
                                vimWord buffEntrySize,
                                vimStringPtr pBuffArray,
                                vimWordPtr pCount,
                                vimBoolPtr pMore);

typedef vimStatus (VIMCALLBACK PVIMENUMERATEMESSAGEITEMS)
                                (vimMsg message,
                                vimEnumRefPtr pPos,
                                vimInt skipCount,
                                vimWordPtr pCount,
                                vimItemDescPtr pDesc,
                                vimSelector selFiltType,
                                vimDataPtr pFiltData,
                                vimBoolPtr pMore);

typedef vimStatus (VIMCALLBACK PVIMENUMERATEMESSAGERECIPIENTS)
                                (vimMsg message,
                                vimSelector selType,
                                vimEnumRefPtr pPos,
                                vimInt skipCount,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrDesc,
                                vimWordPtr pCount,
                                vimBoolPtr pMore);

typedef vimStatus (VIMCALLBACK PVIMENUMERATEMSGATTRVALUES)
                                (vimMsgContainer msgContr,
                                vimSelector selAttr,
                                vimEnumRefPtr pPos,
                                vimInt skipCount,
                                vimWord buffEntrySize,
                                vimStringPtr pBuffArray,
                                vimWordPtr pCount,
                                vimBoolPtr pMore);

typedef vimStatus (VIMCALLBACK PVIMEXTRACTMESSAGE)
                                (vimMsg msg,
                                vimDWord dwFlags,
                                vimStringPtr strFileSpec);

typedef vimStatus (VIMCALLBACK PVIMGETABENTRYATTRIBUTES)
                                (vimAddressBook addressBook,
                                vimRef entryRef,
                                vimStringPtr entryName,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrDesc);

typedef vimStatus (VIMCALLBACK PVIMGETABENTRYNAMEDATTRIBUTES)
                                (vimAddressBook addressBook,
                                vimRef entryRef,
                                vimStringPtr entryName,
                                vimWord attrCount,
                                vimNamedAttrDescPtr pAttrDesc);

typedef vimStatus (VIMCALLBACK PVIMGETCURRENTSUBTREE)
                                (vimAddressBook addressBook,
                                vimRefPtr pRef,
                                vimWord wNameSize,
                                vimStringPtr psName);

typedef vimStatus (VIMCALLBACK PVIMGETDEFAULTSESSIONINFO)
                                (vimWord sizePath,
                                vimStringPtr psPathSpec,
                                vimWord sizeName,
                                vimStringPtr psName);

typedef vimStatus (VIMCALLBACK PVIMGETENTITYNAME)
                                (vimSes session,
                                vimDistinguishedNamePtr pDistName);

typedef vimStatus (VIMCALLBACK PVIMGETMESSAGEHEADER)
                                (vimMsg message,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrDesc);

typedef vimStatus (VIMCALLBACK PVIMGETMESSAGEITEM)
                                (vimMsg message,
                                vimRef refItem,
                                vimStringPtr psConvertType,
                                vimSelector selFlags,
                                vimBuffFileDescPtr pBuffFileDesc );

typedef vimStatus (VIMCALLBACK PVIMINITIALIZE)
                                (vimVoid);

typedef vimStatus (VIMCALLBACK PVIMMARKMESSAGEASREAD)
                                (vimMsgContainer msgContainer,
                                vimRef msgRef);

typedef vimStatus (VIMCALLBACK PVIMMATCHADDRESSBOOK)
                                (vimAddressBook addressBook,
                                vimRef subtreeRef,
                                vimStringPtr subtreeName,
                                vimSelector selAttr,
                                vimDataPtr pData,
                                vimSelector ft,
                                vimRefPtr pRef,
                                vimEnumRefPtr pEnumRef);

typedef vimStatus (VIMCALLBACK PVIMOPENADDRESSBOOK)
                                (vimSes session,
                                vimStringPtr psName,
                                vimAddressBookPtr pAddressBook);

typedef vimStatus (VIMCALLBACK PVIMOPENEXTRACTEDMESSAGE)
                                (vimSes session,
                                vimStringPtr strFileSpec,
                                vimMsgPtr pMsg );

typedef vimStatus (VIMCALLBACK PVIMOPENMESSAGE)
                                (vimMsgContainer msgContainer,
                                vimRef refMessage,
                                vimStringPtr key,
                                vimMsgPtr pMessage);

typedef vimStatus (VIMCALLBACK PVIMOPENMESSAGECONTAINER)
                                (vimSes session,
                                vimStringPtr psContainerName,
                                vimMsgContainerPtr pMsgContainer);

typedef vimStatus (VIMCALLBACK PVIMOPENMESSAGEITEM)
                                (vimMsg message,
                                vimRef refMsgItem,
                                vimStringPtr psConvertType,
                                vimSelector selFlags,
                                vimMsgItemPtr pMsgItem);

typedef vimStatus (VIMCALLBACK PVIMOPENNESTEDMESSAGE)
                                (vimMsg message,
                                vimRef refNested,
                                vimMsgPtr pNestedMessage);

typedef vimStatus (VIMCALLBACK PVIMOPENSESSION)
                                (vimStringPtr psPathSpec,
                                vimStringPtr psName,
                                vimStringPtr psPass,
                                vimDWord dwVersion,
                                vimSelector selCharSet,
                                vimSesPtr pSession);

typedef vimStatus (VIMCALLBACK PVIMQUERYCAPABILITY)
                                (vimSelector selType,
                                vimWord sizeData,
                                vimDataPtr pData);

typedef vimStatus (VIMCALLBACK PVIMQUERYNEWMESSAGES)
                                (vimMsgContainer msgContainer,
                                vimBoolPtr pbNewMessages);

typedef vimStatus (VIMCALLBACK PVIMQUERYUNREADMAILCOUNT)
                                (vimMsgContainer inbox,
                                vimDWordPtr pdwNumUnread);

typedef vimStatus (VIMCALLBACK PVIMREADMESSAGEITEM)
                                (vimMsgItem msgItem,
                                vimBuffFileDescPtr pBuffFileDesc);

typedef vimStatus (VIMCALLBACK PVIMREMOVEADDRESSBOOKENTRY)
                                (vimAddressBook addressBook,
                                vimRef entryRef,
                                vimStringPtr entryName);

typedef vimStatus (VIMCALLBACK PVIMREMOVEGROUPMEMBER)
                                (vimAddressBook addressBook,
                                vimRef entryRef,
                                vimStringPtr entryName,
                                vimDistinguishedNamePtr pDisName);

typedef vimStatus (VIMCALLBACK PVIMREMOVEMESSAGE)
                                (vimMsgContainer msgContainer,
                                vimRef msgRef);

typedef vimStatus (VIMCALLBACK PVIMSENDMESSAGE)
                                (vimMsg message,
                                vimDataPtr pCallbackParam,
                                vimSendCallback callBack );

typedef vimStatus (VIMCALLBACK PVIMSETABENTRYATTRIBUTES)
                                (vimAddressBook addressBook,
                                vimRef entryRef,
                                vimStringPtr entryName,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrDesc);

typedef vimStatus (VIMCALLBACK PVIMSETABENTRYNAMEDATTRIBUTES)
                                (vimAddressBook addressBook,
                                vimRef entryRef,
                                vimStringPtr entryName,
                                vimWord attrCount,
                                vimNamedAttrDescPtr pAttrDesc);

typedef vimStatus (VIMCALLBACK PVIMSETCURRENTSUBTREE)
                                (vimAddressBook addressBook,
                                vimRef subtreeRef,
                                vimStringPtr subtreeName);

typedef vimStatus (VIMCALLBACK PVIMSETMESSAGEHEADER)
                                (vimMsg message,
                                vimSelector selAttr,
                                vimWord wAttrSize,
                                vimDataPtr pAttr);

typedef vimStatus (VIMCALLBACK PVIMSETMESSAGEITEM)
                                (vimMsg message,
                                vimSelector selClass,
                                vimStringPtr strType,
                                vimSelector selFlags,
                                vimStringPtr strName,
                                vimBuffFileDescPtr pDesc );

typedef vimStatus (VIMCALLBACK PVIMSETMESSAGERECIPIENT)
                                (vimMsg message,
                                vimSelector selClass,
                                vimRecipientPtr pRecip);

typedef vimStatus (VIMCALLBACK PVIMSTATUSTEXT)
                                (vimSes session,
                                vimStatus status,
                                vimWord wTextSize,
                                vimStringPtr pText,
                                vimWord wExtTextSize,
                                vimStringPtr pExtText);

typedef vimStatus (VIMCALLBACK PVIMTERMINATE)
                                (vimVoid);

typedef vimStatus (VIMCALLBACK PVIMVERIFYMESSAGESIGNATURE)
                                (vimMsg message,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrDesc);

/*  Lotus extension VIM functions pointer datatypes. */

typedef vimStatus (VIMCALLBACK PVIMOPENGATEWAYSESSION)
                                (vimStringPtr psPathSpec,
                                vimStringPtr psPostOffice,
                                vimStringPtr psPass,
                                vimDWord dwVersion,
                                vimSelector selCharSet,
                                vimSesPtr pSession);

typedef vimStatus (VIMCALLBACK PVIMREMOVEMESSAGECATEGORY)
                                (vimMsgContainer msgContr,
                                vimRef msgRef,
                                vimStringPtr psCatName);

typedef vimStatus (VIMCALLBACK PVIMSETMESSAGECATEGORY)
                                (vimMsgContainer msgContr,
                                vimRef msgRef,
                                vimStringPtr psCatName);

typedef vimStatus (VIMCALLBACK PVIMGETSESSIONATTRIBUTES)
                                (vimSes session,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrs);

typedef vimStatus (VIMCALLBACK PVIMSETSESSIONATTRIBUTES)
                                (vimSes session,
                                vimWord attrCount,
                                vimAttrDescPtr pAttrs);
                                
typedef vimStatus (VIMCALLBACK PVIMCREATEMESSAGEEXTENDED)
                                (vimSes Session,
                                vimMsgContainer Container,
                                vimStringPtr psType,
                                vimMsgPtr pMessage);

typedef vimStatus (VIMCALLBACK PVIMCONVERTUNIDTOREF)
                                (vimMsgContainer Container,
                                vimStringPtr pUNIDStr, 
                                vimRefPtr retMsgRef);

typedef vimStatus (VIMCALLBACK PVIMOPENSHAREDCONTAINER)
                                (vimSes session,
                                vimStringPtr psContainerName,
                                vimMsgContainerPtr pMsgContainer);

#endif  /* __VIMEXT_H */

#ifdef __cplusplus
}
#endif

