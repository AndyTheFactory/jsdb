/* Default filename for VIM DLL */
#ifdef __WIN32__
#define VIMDLL       "VIM32.DLL"
#else
#define VIMDLL       "VIM.DLL"
#endif
#define FVIMRemoveMessage PVIMREMOVEMESSAGE
#define FVIMCloseMessage PVIMCLOSEMESSAGE
#define FVIMCloseSession PVIMCLOSESESSION
#define FVIMCreateMessage PVIMCREATEMESSAGE
#define FVIMSetMessageHeader PVIMSETMESSAGEHEADER
#define FVIMSetMessageItem PVIMSETMESSAGEITEM
#define FVIMSetMessageRecipient PVIMSETMESSAGERECIPIENT
#define FVIMSendMessage PVIMSENDMESSAGE
#define FVIMGetDefaultSessionInfo PVIMGETDEFAULTSESSIONINFO
#define FVIMOpenSession PVIMOPENSESSION
#define FVIMInitialize PVIMINITIALIZE
#define FVIMTerminate PVIMTERMINATE
#define FVIMCloseMessage PVIMCLOSEMESSAGE
#define FVIMCloseMessageContainer PVIMCLOSEMESSAGECONTAINER
#define FVIMEnumerateMessages PVIMENUMERATEMESSAGES
#define FVIMEnumerateMessageItems PVIMENUMERATEMESSAGEITEMS
#define FVIMMarkMessageAsRead PVIMMARKMESSAGEASREAD
#define FVIMOpenMessage PVIMOPENMESSAGE
#define FVIMOpenMessageContainer PVIMOPENMESSAGECONTAINER
#define FVIMOpenMessageItem PVIMOPENMESSAGEITEM
#define FVIMGetMessageItem PVIMGETMESSAGEITEM

/*
typedef vimStatus (VIMAPIENTRY *FVIMRemoveMessage)
                 (    vimMsgContainer msgContainer,
                                           vimRef msgRef);

typedef vimStatus (VIMAPIENTRY *FVIMCloseMessage)( vimMsg message);

typedef vimStatus (VIMAPIENTRY *FVIMCloseSession)( vimSes session);

typedef vimStatus (VIMAPIENTRY *FVIMCreateMessage)(    vimSes session,
                                           vimStringPtr psType,
                                           vimMsgPtr pMessage);

typedef vimStatus (VIMAPIENTRY *FVIMSetMessageHeader)( vimMsg message,
                                           vimSelector selAttr,
                                           vimWord wAttrSize,
                                           vimDataPtr pAttr);

typedef vimStatus (VIMAPIENTRY *FVIMSetMessageItem)(   vimMsg message,
                                           vimSelector selClass,
                                           vimStringPtr strType,
                                           vimSelector selFlags,
                                           vimStringPtr strName,
                                           vimBuffFileDescPtr pDesc );

typedef vimStatus (VIMAPIENTRY *FVIMSetMessageRecipient)(  vimMsg message,
                                               vimSelector selClass,
                                               vimRecipientPtr pRecip);

typedef vimStatus (VIMAPIENTRY *FVIMSendMessage)(  vimMsg message,
                                       vimDataPtr pCallbackParam,
                                       vimSendCallback callBack );

typedef vimStatus (VIMAPIENTRY *FVIMGetDefaultSessionInfo)(    vimWord sizePath,
                                                   vimStringPtr psPathSpec,
                                                   vimWord sizeName,
                                                   vimStringPtr psName);

typedef vimStatus (VIMAPIENTRY *FVIMOpenSession)(  vimStringPtr psPathSpec,
                                       vimStringPtr psName,
                                       vimStringPtr psPass,
                                       vimDWord dwVersion,
                                       vimSelector selCharSet,
                                       vimSesPtr pSession);

typedef vimStatus (VIMAPIENTRY *FVIMInitialize)( vimVoid );

typedef vimStatus (VIMAPIENTRY *FVIMTerminate)( vimVoid );


 
typedef vimStatus (VIMAPIENTRY *FVIMCloseMessage)( vimMsg message);

typedef vimStatus (VIMAPIENTRY *FVIMCloseMessageContainer)( vimMsgContainer msgContainer);


typedef vimStatus (VIMAPIENTRY *FVIMEnumerateMessages)(    vimMsgContainer msgContainer,
                                               vimEnumRefPtr pPos,
                                               vimInt skipCount,
                                               vimWord attribCount,
                                               vimAttrDescPtr pAttribDesc,
                                               vimWordPtr pMsgCount,
                                               vimSelector selFiltType,
                                               vimDataPtr pFiltData,
                                               vimDWord flags,
                                               vimBoolPtr pMore);

 
typedef vimStatus (VIMAPIENTRY *FVIMEnumerateMessageItems)(    vimMsg message,
                                                   vimEnumRefPtr pPos,
                                                   vimInt skipCount,
                                                   vimWordPtr pCount,
                                                   vimItemDescPtr pDesc,
                                                   vimSelector selFiltType,
                                                   vimDataPtr pFiltData,
                                                   vimBoolPtr pMore);

typedef vimStatus (VIMAPIENTRY *FVIMMarkMessageAsRead)(    vimMsgContainer msgContainer,
                                               vimRef msgRef );

typedef vimStatus (VIMAPIENTRY *FVIMOpenMessage)(  vimMsgContainer msgContainer,
                                       vimRef refMessage,
                                       vimStringPtr key,
                                       vimMsgPtr pMessage);

typedef vimStatus (VIMAPIENTRY *FVIMOpenMessageContainer)( vimSes session,
                                               vimStringPtr psContainerName,
                                               vimMsgContainerPtr pMsgContainer);

typedef vimStatus (VIMAPIENTRY *FVIMOpenMessageItem)(  vimMsg message,
                                           vimRef refMsgItem,
                                           vimStringPtr psConvertType,
                                           vimSelector selFlags,
                                           vimMsgItemPtr pMsgItem);

typedef vimStatus (VIMAPIENTRY *FVIMGetMessageItem)(   vimMsg message,
                                           vimRef refItem,
                                           vimStringPtr psConvertType,
                                           vimSelector selFlags,
                                           vimBuffFileDescPtr pBuffFileDesc );

*/
