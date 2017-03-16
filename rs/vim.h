#ifdef __cplusplus
extern "C" {
#endif

/* VIM.H is the public header file for Vendor Independent Messaging (VIM) */

#ifndef __VIM_INCLUDED
#define __VIM_INCLUDED

#ifdef __cplusplus		
extern "C" {
#endif

/*
   Include local implementation bindings.
*/
#include "vimlcl.h"

/*
   VIM Version (100 = 1.00)
*/
#define    VIM_CURRENT_VERSION             100L /* VIM Spec Version */
#define    VIM_IMPLEMENTATION_VERSION      101L	/* VIM Implementation Version */

/*
   VIM datatypes.
*/
/* vimvoid should not be typedef because a prototype of
 *	(datatype) func ( vimVoid );
 * because the null param list is special cased void and
 * is not defined to work with some c and c++ compilers 
 */
#define    vimVoid VIM_VOID 
typedef    vimVoid VIM_FAR * VIM_PTR   vimVoidPtr;

typedef unsigned short int             vimBool;
typedef vimBool VIM_FAR * VIM_PTR      vimBoolPtr;

typedef unsigned short int             vimWord;
typedef vimWord    VIM_FAR * VIM_PTR   vimWordPtr;

typedef unsigned long int              vimDWord;
typedef vimDWord VIM_FAR * VIM_PTR     vimDWordPtr;

typedef signed long int                vimInt;
typedef vimInt VIM_FAR * VIM_PTR       vimIntPtr;

#ifndef __VIM_ISTRING
typedef char                           vimString;
#else
typedef struct
   {
   vimWord Script;
   vimWord Length;
   char    String;
   } vimString;
#endif
typedef vimString VIM_FAR * VIM_PTR    vimStringPtr;
typedef vimStringPtr VIM_FAR * VIM_PTR vimStringPtrPtr;

typedef vimVoidPtr                     vimSes;
typedef vimSes VIM_FAR * VIM_PTR       vimSesPtr;

typedef struct {
   vimDWord Ref[2];
} vimRef;
typedef vimRef VIM_FAR * VIM_PTR       vimRefPtr;

typedef struct {
   vimDWord Ref[2];
} vimEnumRef;
typedef vimEnumRef VIM_FAR * VIM_PTR   vimEnumRefPtr;

typedef vimVoidPtr                     vimMsgContainer;
typedef vimMsgContainer VIM_FAR * VIM_PTR      vimMsgContainerPtr;

typedef vimVoidPtr                     vimMsg;
typedef vimMsg VIM_FAR * VIM_PTR       vimMsgPtr;

typedef vimVoidPtr                     vimAddressBook;
typedef vimAddressBook VIM_FAR * VIM_PTR       vimAddressBookPtr;

typedef vimVoidPtr                     vimMsgItem;
typedef vimMsgItem VIM_FAR * VIM_PTR   vimMsgItemPtr;

typedef vimDWord                       vimSelector;
typedef vimSelector VIM_FAR * VIM_PTR  vimSelectorPtr;

typedef vimDWord                       vimStatus;
/* vimStatusPtr is used by VIMSEL_FAILURE_CODE */
typedef vimStatus VIM_FAR * VIM_PTR    vimStatusPtr;

typedef vimVoidPtr                     vimDataPtr;

/*
   Distinguished Entity Name.
*/
typedef struct {
   vimSelector     Type;               /* Type of name (e.g., VIMSEL_X500) */
   vimString       AddressBook[256];   /* Address book name                */
   vimString       Value[1024];        /* Entity name                      */
} vimDistinguishedName;
typedef vimDistinguishedName VIM_FAR * VIM_PTR vimDistinguishedNamePtr;

typedef struct {
   vimSelector     Type;           /* Type of address (e.g., VIMSEL_X400)  */
   vimString       Value[1024];    /* Address string                       */
}  vimAddress;
typedef vimAddress VIM_FAR * VIM_PTR   vimAddressPtr;

typedef struct {
   vimSelector             Type;       /* VIMSEL_ENTITY, GROUP, etc.       */
   vimDistinguishedName    DName;      /* Name                             */
   vimAddress              Address;    /* Address                          */
}  vimRecipient;
typedef vimRecipient VIM_FAR * VIM_PTR vimRecipientPtr;

/*
   Callback function prototype for VIMSendMessage recipient handling.
*/
typedef vimBool (VIMCALLBACK vimSendCallback) (
                                       /* Return VIM_FALSE to cancel the   */
                                       /* message (VIM_TRUE to proceed as  */
                                       /* specified by the pSelection parm */
   vimDataPtr      callbackRoutineParm,/*Passed thru from VIMSendMessage   */
   vimStatus       status,             /* Reason why routine called        */
   vimRecipientPtr pRecip,             /* Recipient in question            */
   vimWord         wChoicesCount,      /* Number of entries in pChoices    */
   vimRecipientPtr pChoices,           /* Array of recipients to choose from */
   vimWordPtr      pSelection          /* Pointer to return result (index  */
                                       /* into pChoices, or VIM_SKIP_RECIP */
                                       /* to skip this recipient without   */
                                       /* cancelling the message)          */
);

/*
   Attribute Descriptor.
*/
typedef struct {
   vimSelector     Attr;           /* Attribute selector                   */
   vimDWord        Size;           /* Buffer or buffer array entry size    */
   vimDataPtr      Buffer;         /* Buffer or buffer array pointer       */
   } vimAttrDesc;
typedef vimAttrDesc VIM_FAR * VIM_PTR vimAttrDescPtr;

/*
   Named Attribute Descriptor.
*/
typedef struct {
   vimStringPtr    Attr;           /* Attribute name string pointer        */
   vimDWord        Size;           /* Buffer or buffer array entry size    */
   vimDataPtr      Buffer;         /* Buffer or buffer array pointer       */
   } vimNamedAttrDesc;
typedef vimNamedAttrDesc VIM_FAR * VIM_PTR vimNamedAttrDescPtr;

/*
   Item Descriptor.
*/
typedef struct {
   vimSelector Class;    /* VIMSEL_NOTE_PART,VIMSEL_ATTACH,VIMSEL_APP_DEFINED */
   vimString   Type[64]; /* Item Type */
   vimString   Name[256];/* Title for note part OR file name for attachments */
   vimString   Path[256];/* Holds pathname for attachments */
   vimDWord    Size;     /* Item size */
   vimRef      Ref;      /* Item ref */
} vimItemDesc;
typedef vimItemDesc VIM_FAR * VIM_PTR vimItemDescPtr;

/*
   File Buffer Descriptor.
*/
typedef struct {
   vimDWord        Size;           /* Buffer size or number of bytes to read*/
   vimDataPtr      Buffer;         /* Buffer pointer                       */
                                   /*  (must be NULL if FileName used)     */
   vimDWord        Offset;         /* Starting offset at which to start    */
                                   /*  reading/writing                     */
   vimStringPtr    FileName;       /* File name string                     */
} vimBuffFileDesc;
typedef vimBuffFileDesc VIM_FAR * VIM_PTR vimBuffFileDescPtr;

/*
   Date and Time.
*/

typedef struct {
   vimSelector     Zone;       /* Time zone (VIMSEL_LOCAL means local time,*/
                               /*  VIMSEL_GMT means GMT, other values TBD  */
   vimDWord        Date;       /* Date                                     */
                               /*  low-order byte is day (1 to 31)         */
                               /*  next-order byte is month (1 to 12)      */
                               /*  high-order word is year (1980 to 2099)  */
                               /*  VIM_UNKNOWN_DATE means undefined and    */
                               /*  Time is ignored                         */
   vimDWord        Time;       /* Time                                     */
                               /*  low-order byte is 0.01 second (0 to 99) */
                               /*  next-order byte is second (0 to 59)     */
                               /*  next-order byte is minute (0 to 59)     */
                               /*  high-order byte is hour (0 to 23)       */
                               /*  VIM_UNKNOWN_TIME means undefined time   */
                               /*  even though date may be known.          */
} vimDate;

typedef vimDate VIM_FAR * VIM_PTR vimDatePtr;

/*
   Date Range.
*/
typedef struct {
   vimDate         Start;      /* Starting date and time */
   vimDate         Stop;       /* Ending date and time */
} vimDateRange;

typedef vimDateRange VIM_FAR * VIM_PTR vimDateRangePtr;

/*
   vimSelector values.
*/
#define VIMSEL_ADDRESS                 1L
#define VIMSEL_ADMINISTRATOR           2L
#define VIMSEL_ALL_NOTE_PARTS_SUPP     3L
#define VIMSEL_APP_DEFINED             4L
#define VIMSEL_APPLESINGLE             5L
#define VIMSEL_ATTACH                  6L
#define VIMSEL_ATTACH_DIRS             7L
#define VIMSEL_ATTACH_TYPE_SUPP        8L
#define VIMSEL_BCC                     9L
#define VIMSEL_CC                      10L
#define VIMSEL_CCMAIL                  11L
#define VIMSEL_CERTIFIER               12L
#define VIMSEL_CLASS                   13L
#define VIMSEL_COMMENTS                14L
#define VIMSEL_CONVERSATION_ID         15L
#define VIMSEL_CP1252                  16L
#define VIMSEL_CP437                   17L
#define VIMSEL_CP850                   18L
#define VIMSEL_DATE                    19L
#define VIMSEL_DELIVERY_DATE           20L
#define VIMSEL_DELIVERY_REPORT         21L
#define VIMSEL_DISPLAY_NAME            22L
#define VIMSEL_DRAFT                   23L
#define VIMSEL_ENCRYPT                 24L
#define VIMSEL_ENCRYPT_WITH_KEY        25L
#define VIMSEL_ENTITY                  26L
#define VIMSEL_EXPIRATION_DATE         27L
#define VIMSEL_FAILED                  28L
#define VIMSEL_FAILURE_CODE            29L
#define VIMSEL_FAILURE_REASON          30L
#define VIMSEL_FAX                     31L
#define VIMSEL_FIRST_NAME              32L
#define VIMSEL_FORWARD                 33L
#define VIMSEL_FROM                    34L
#define VIMSEL_FROM_NAME               35L
#define VIMSEL_FULL                    36L
#define VIMSEL_GMT                     37L
#define VIMSEL_GROUP                   38L
#define VIMSEL_IMAG                    39L
#define VIMSEL_IN_REPLY_TO             40L
#define VIMSEL_ISTRING                 41L
#define VIMSEL_KEYWORD                 42L
#define VIMSEL_LAST_NAME               43L
#define VIMSEL_LMBCS                   44L
#define VIMSEL_LOCAL                   45L
#define VIMSEL_MAX_SUBJECT_LEN         46L
#define VIMSEL_MAX_TEXT_LEN            47L
#define VIMSEL_MAX_TYPE_LEN            48L
#define VIMSEL_MHS                     49L
#define VIMSEL_MIDDLE_NAME             50L
#define VIMSEL_MOVIE                   51L
#define VIMSEL_MSGMGR                  52L
#define VIMSEL_NAME                    53L
#define VIMSEL_NAME_REQUIRED           54L
#define VIMSEL_NATIVE                  55L
#define VIMSEL_NESTED_MSG              56L
#define VIMSEL_NESTING_DEPTH           57L
#define VIMSEL_NO_FILTER               58L
#define VIMSEL_NONDELIVERY_CONTENTS    59L
#define VIMSEL_NONDELIVERY_REPORT      60L
#define VIMSEL_NOT_SUPPORTED           61L
#define VIMSEL_NOTE_PART               62L
#define VIMSEL_NOTES                   63L
#define VIMSEL_NSTD_DERIVED_FORWRDS    64L
#define VIMSEL_NSTD_DERIVED_REPLIES    65L
#define VIMSEL_OCE                     66L
#define VIMSEL_PARTIAL                 67L
#define VIMSEL_PASS_REQUIRED           68L
#define VIMSEL_PATH_REQUIRED           69L
#define VIMSEL_PICT                    70L
#define VIMSEL_PRIORITY                71L
#define VIMSEL_PRODUCT                 72L
#define VIMSEL_RDN                     73L
#define VIMSEL_READ_DATE               74L
#define VIMSEL_RECIPIENT               75L
#define VIMSEL_REF                     76L
#define VIMSEL_REPLY                   77L
#define VIMSEL_RESPOND_BY              78L
#define VIMSEL_RETURN_RECEIPT          79L
#define VIMSEL_RTF                     80L
#define VIMSEL_SAVE                    81L
#define VIMSEL_SENSITIVITY             82L
#define VIMSEL_SENT                    83L
#define VIMSEL_SIGN                    84L
#define VIMSEL_SIGNER                  85L
#define VIMSEL_SORT_ORDER              86L
#define VIMSEL_SORTED_LASTNAME         87L
#define VIMSEL_SORTED_NAME             88L
#define VIMSEL_SORTED_RDN              89L
#define VIMSEL_STYLED                  90L
#define VIMSEL_SUBJECT                 91L
#define VIMSEL_SUBJECT_LEN             92L
#define VIMSEL_SUBTREE                 93L
#define VIMSEL_SUCCEEDED               94L
#define VIMSEL_SUPP_WITH_CONV          95L
#define VIMSEL_SUPP_WITHOUT_CONV       96L
#define VIMSEL_TO                      97L
#define VIMSEL_TYPE                    98L
#define VIMSEL_UNICODE                 99L
#define VIMSEL_UNIQUE_MSG_ID           100L
#define VIMSEL_UNKNOWN_RECIP_TYPE      101L
#define VIMSEL_UNKNOWN_SORT            102L
#define VIMSEL_UNREAD_MAIL             103L
#define VIMSEL_UNSORTED                104L
#define VIMSEL_UNWRAPPED_TEXT          105L
#define VIMSEL_VERSION                 106L
#define VIMSEL_X400                    107L
#define VIMSEL_X500                    108L

/*
   Symbolic Names
*/
#define VIM_DLR                    "VIM_DLR"
#define VIM_FAX                    "VIM_FAX"
#define VIM_IMAG                   "VIM_IMAG"
#define VIM_MAIL                   "VIM_MAIL"
#define VIM_MOVIE                  "VIM_MOVIE"
#define VIM_NDLR                   "VIM_NDLR"
#define VIM_PICT                   "VIM_PICT"
#define VIM_PRIVATE                "VIM_PRIVATE"
#define VIM_PUBLIC                 "VIM_PUBLIC"
#define VIM_RTF                    "VIM_RTF"
#define VIM_RTRC                   "VIM_RTRC"
#define VIM_STYLED                 "VIM_STYLED"
#define VIM_TEXT                   "VIM_TEXT"
#define VIM_UNWRAPPED_TEXT         "VIM_UNWRAPPED_TEXT"


/*
   TRUE/FALSE.
*/
#define VIM_FALSE                  0
#define VIM_TRUE                   1

/*
   Priority values for VIMSEL_PRIORITY.
*/
#define VIM_LOW_PRIORITY               0
#define VIM_NORMAL_PRIORITY            1
#define VIM_HIGH_PRIORITY              2

/*
   Bits for dwFlags parameter of VIMCreateDerrivedMessage call when
   VIMSEL_FORWARD is used.
*/
#define VIM_HISTORY                    0x00000001L

/*
   Bits for dwFlags parameter of VIMCreateDerrivedMessage call when
   VIMSEL_REPLY is used.
*/
#define VIM_INHERIT_CONTENTS           0x00000002L
#define VIM_ALL_RECIPIENTS             0x00000004L

#define VIM_UNREADONLY                 0x00000001L

#define VIM_NORMAL_SENS                0
#define VIM_PRIVATE_SENS               1
#define VIM_PERSONAL_SENS              2
#define VIM_CO_CONFID_SENS             3
#define VIM_APP_DEF_SENS               16384

/*
   Bits for flags parameter of VIMExtract Message call
*/
#define VIM_EXTRACT_REF                0x00000001

#define VIM_SKIP_RECIP                 ((vimWord)(-1))

#define VIM_UNKNOWN_DATE               ((vimDWord)(-1))
#define VIM_UNKNOWN_TIME               ((vimDWord)(-1))

/*
   Macros for accessing the VIM error or the extended error.
*/
#define VIM_STATUS(l)         ( (vimWord) (((vimStatus) (l) >> 0)  & 0xFFFF) )
#define VIM_EXT_STATUS(l)     ( (vimWord) (((vimStatus) (l) >> 16) & 0xFFFF) )

/*
   Macros for manipulating a vimRef or vimEnumRef.
*/
#define VIM_UND_REF(x)     ( (x).Ref[0] = (x).Ref[1] = 0L )
#define VIM_ROOT_REF(x)    ( (x).Ref[0] = 0L, (x).Ref[1] = -1L )
#define VIM_PARENT_REF(x)  ( (x).Ref[0] = 0L, (x).Ref[1] = -2L )

/*
   Status returns.
*/
#define VIMSTS_SUCCESS                     0
#define VIMSTS_FAILURE                     1
#define VIMSTS_FATAL                       2

#define VIMSTS_ALL_PARAMS_REQUIRED         3
#define VIMSTS_ATTACHMENT_NOT_FOUND        4
#define VIMSTS_BAD_PARAM                   5
#define VIMSTS_BUF_TOO_SMALL               6
#define VIMSTS_CONV_NOT_SUPPORTED          7
#define VIMSTS_INSUFFICIENT_MEMORY         8
#define VIMSTS_INVALID_CONFIGURATION       9
#define VIMSTS_INVALID_OBJECT              10
#define VIMSTS_INVALID_PASSWORD            11
#define VIMSTS_INVALID_SELECTOR            12
#define VIMSTS_INVALID_SIGNATURE           13
#define VIMSTS_NAME_EXISTS                 14
#define VIMSTS_NAME_NOT_FOUND              15
#define VIMSTS_NOT_SUPPORTED               16
#define VIMSTS_NO_COMMON_CERTIFICATES      17
#define VIMSTS_NO_DEFAULT                  18
#define VIMSTS_NO_MATCH                    19
#define VIMSTS_NO_SIGNATURE                20
#define VIMSTS_NO_SUCH_ATTRIBUTE           21
#define VIMSTS_OPEN_FAILURE                22
#define VIMSTS_PASS_REQUIRED               23
#define VIMSTS_READ_FAILURE                24
#define VIMSTS_UNSUP_TYPE                  25
#define VIMSTS_UNSUP_VERSION               26
#define VIMSTS_WRITE_FAILURE               27
#define VIMSTS_UNABLE_CREATE_DRAFT         28


/*
   Function Prototypes
*/

vimStatus VIMAPIENTRY VIMAddGroupMember(   vimAddressBook addressBook,
                                           vimRef entryRef,
                                           vimStringPtr entryName,
                                           vimDistinguishedNamePtr pDisName);

vimStatus VIMAPIENTRY VIMCloseAddressBook( vimAddressBook addrBook);

vimStatus VIMAPIENTRY VIMCloseMessage( vimMsg message);

vimStatus VIMAPIENTRY VIMCloseMessageContainer( vimMsgContainer msgContainer);

vimStatus VIMAPIENTRY VIMCloseMessageItem( vimMsgItem msgItem);

vimStatus VIMAPIENTRY VIMCloseSession( vimSes session);

vimStatus VIMAPIENTRY VIMCreateAddressBookEntry(   vimAddressBook addressBook,
                                                   vimSelector entryType,
                                                   vimWord attrCount,
                                                   vimAttrDescPtr pAttrDesc);

vimStatus VIMAPIENTRY VIMCreateDerivedMailMessage( vimMsg message,
                                                   vimSelector selMsgType,
                                                   vimDWord dwFlags,
                                                   vimMsgPtr pMessage);

vimStatus VIMAPIENTRY VIMCreateMessage(    vimSes session,
                                           vimStringPtr psType,
                                           vimMsgPtr pMessage);

vimStatus VIMAPIENTRY VIMEnumerateABEntryAttrNames(
                                   vimAddressBook addressBook,
                                   vimRef entryRef,
                                   vimStringPtr entryName,
                                   vimEnumRefPtr pPos,
                                   vimInt skipCount,
                                   vimWord buffEntrySize,
                                   vimStringPtr pBuffArray,
                                   vimWordPtr pCount,
                                   vimBoolPtr pMore);

vimStatus VIMAPIENTRY VIMEnumerateAddressBookEntries(
                                   vimAddressBook addressBook,
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

vimStatus VIMAPIENTRY VIMEnumerateAddressBooks(    vimSes session,
                                                   vimEnumRefPtr pPos,
                                                   vimInt skipCount,
                                                   vimWord attrCount,
                                                   vimAttrDescPtr pAttrDesc,
                                                   vimWordPtr pCount,
                                                   vimBoolPtr pMore);

vimStatus VIMAPIENTRY VIMEnumerateGroupMembers(    vimAddressBook addressBook,
                                                   vimRef entryRef,
                                                   vimStringPtr entryName,
                                                   vimEnumRefPtr pPos,
                                                   vimInt skipCount,
                                                   vimWord attrCount,
                                                   vimAttrDescPtr pAttrDesc,
                                                   vimWordPtr pCount,
                                                   vimBoolPtr pMore);

vimStatus VIMAPIENTRY VIMEnumerateMessages(    vimMsgContainer msgContainer,
                                               vimEnumRefPtr pPos,
                                               vimInt skipCount,
                                               vimWord attribCount,
                                               vimAttrDescPtr pAttribDesc,
                                               vimWordPtr pMsgCount,
                                               vimSelector selFiltType,
                                               vimDataPtr pFiltData,
                                               vimDWord flags,
                                               vimBoolPtr pMore);

vimStatus VIMAPIENTRY VIMEnumerateMessageHeaderAttrs(
                                               vimMsg msg,
                                               vimSelector selAttr,
                                               vimEnumRefPtr pPos,
                                               vimInt skipCount,
                                               vimWord buffEntrySize,
                                               vimStringPtr pBuffArray,
                                               vimWordPtr pCount,
                                               vimBoolPtr pMore);

vimStatus VIMAPIENTRY VIMEnumerateMessageItems(    vimMsg message,
                                                   vimEnumRefPtr pPos,
                                                   vimInt skipCount,
                                                   vimWordPtr pCount,
                                                   vimItemDescPtr pDesc,
                                                   vimSelector selFiltType,
                                                   vimDataPtr pFiltData,
                                                   vimBoolPtr pMore);

vimStatus VIMAPIENTRY VIMEnumerateMessageRecipients(
                                               vimMsg message,
                                               vimSelector selType,
                                               vimEnumRefPtr pPos,
                                               vimInt skipCount,
                                               vimWord attrCount,
                                               vimAttrDescPtr pAttrDesc,
                                               vimWordPtr pCount,
                                               vimBoolPtr pMore);

vimStatus VIMAPIENTRY VIMEnumerateMsgAttrValues(
                                               vimMsgContainer msgContr,
                                               vimSelector selAttr,
                                               vimEnumRefPtr pPos,
                                               vimInt skipCount,
                                               vimWord buffEntrySize,
                                               vimStringPtr pBuffArray,
                                               vimWordPtr pCount,
                                               vimBoolPtr pMore);

vimStatus VIMAPIENTRY VIMExtractMessage(   vimMsg msg,
                                           vimDWord dwFlags,
                                           vimStringPtr strFileSpec );

vimStatus VIMAPIENTRY VIMGetABEntryAttributes( vimAddressBook addressBook,
                                               vimRef entryRef,
                                               vimStringPtr entryName,
                                               vimWord attrCount,
                                               vimAttrDescPtr pAttrDesc);

vimStatus VIMAPIENTRY VIMGetABEntryNamedAttributes(
                                               vimAddressBook addressBook,
                                               vimRef entryRef,
                                               vimStringPtr entryName,
                                               vimWord attrCount,
                                               vimNamedAttrDescPtr pAttrDesc);

vimStatus VIMAPIENTRY VIMGetCurrentSubtree(    vimAddressBook addressBook,
                                               vimRefPtr pRef,
                                               vimWord wNameSize,
                                               vimStringPtr psName);

vimStatus VIMAPIENTRY VIMGetDefaultSessionInfo(    vimWord sizePath,
                                                   vimStringPtr psPathSpec,
                                                   vimWord sizeName,
                                                   vimStringPtr psName);

vimStatus VIMAPIENTRY VIMGetEntityName(    vimSes session,
                                           vimDistinguishedNamePtr pDistName);

vimStatus VIMAPIENTRY VIMGetMessageHeader( vimMsg message,
                                           vimWord attrCount,
                                           vimAttrDescPtr pAttrDesc);

vimStatus VIMAPIENTRY VIMGetMessageItem(   vimMsg message,
                                           vimRef refItem,
                                           vimStringPtr psConvertType,
                                           vimSelector selFlags,
                                           vimBuffFileDescPtr pBuffFileDesc );

vimStatus VIMAPIENTRY VIMInitialize( vimVoid );

vimStatus VIMAPIENTRY VIMMarkMessageAsRead(    vimMsgContainer msgContainer,
                                               vimRef msgRef );

vimStatus VIMAPIENTRY VIMMatchAddressBook( vimAddressBook addressBook,
                                           vimRef subtreeRef,
                                           vimStringPtr subtreeName,
                                           vimSelector selAttr,
                                           vimDataPtr pData,
                                           vimSelector ft,
                                           vimRefPtr pRef,
                                           vimEnumRefPtr pEnumRef);

vimStatus VIMAPIENTRY VIMOpenAddressBook(  vimSes session,
                                           vimStringPtr psName,
                                           vimAddressBookPtr pAddressBook);

vimStatus VIMAPIENTRY VIMOpenExtractedMessage( vimSes session,
                                               vimStringPtr strFileSpec,
                                               vimMsgPtr pMsg );

vimStatus VIMAPIENTRY VIMOpenMessage(  vimMsgContainer msgContainer,
                                       vimRef refMessage,
                                       vimStringPtr key,
                                       vimMsgPtr pMessage);

vimStatus VIMAPIENTRY VIMOpenMessageContainer( vimSes session,
                                               vimStringPtr psContainerName,
                                               vimMsgContainerPtr pMsgContainer);

vimStatus VIMAPIENTRY VIMOpenMessageItem(  vimMsg message,
                                           vimRef refMsgItem,
                                           vimStringPtr psConvertType,
                                           vimSelector selFlags,
                                           vimMsgItemPtr pMsgItem);

vimStatus VIMAPIENTRY VIMOpenNestedMessage(    vimMsg message,
                                               vimRef refNested,
                                               vimMsgPtr pNestedMessage);

vimStatus VIMAPIENTRY VIMOpenSession(  vimStringPtr psPathSpec,
                                       vimStringPtr psName,
                                       vimStringPtr psPass,
                                       vimDWord dwVersion,
                                       vimSelector selCharSet,
                                       vimSesPtr pSession);

vimStatus VIMAPIENTRY VIMQueryCapability(  vimSelector selType,
                                           vimWord sizeData,
                                           vimDataPtr pData);

vimStatus VIMAPIENTRY VIMQueryNewMessages( vimMsgContainer msgContainer,
                                           vimBoolPtr pbNewMessages);

vimStatus VIMAPIENTRY VIMQueryUnreadMailCount( vimMsgContainer inbox,
                                               vimDWordPtr pdwNumUnread);

vimStatus VIMAPIENTRY VIMReadMessageItem(  vimMsgItem msgItem,
                                           vimBuffFileDescPtr pBuffFileDesc);

vimStatus VIMAPIENTRY VIMRemoveAddressBookEntry(
                                       vimAddressBook addressBook,
                                       vimRef entryRef,
                                       vimStringPtr entryName);

vimStatus VIMAPIENTRY VIMRemoveGroupMember(
                                       vimAddressBook addressBook,
                                       vimRef entryRef,
                                       vimStringPtr entryName,
                                       vimDistinguishedNamePtr pDisName);

vimStatus VIMAPIENTRY VIMRemoveMessage(    vimMsgContainer msgContainer,
                                           vimRef msgRef);

vimStatus VIMAPIENTRY VIMSendMessage(  vimMsg message,
                                       vimDataPtr pCallbackParam,
                                       vimSendCallback callBack );

vimStatus VIMAPIENTRY VIMSetABEntryAttributes( vimAddressBook addressBook,
                                               vimRef entryRef,
                                               vimStringPtr entryName,
                                               vimWord attrCount,
                                               vimAttrDescPtr pAttrDesc);

vimStatus VIMAPIENTRY VIMSetABEntryNamedAttributes(
                                       vimAddressBook addressBook,
                                       vimRef entryRef,
                                       vimStringPtr entryName,
                                       vimWord attrCount,
                                       vimNamedAttrDescPtr pAttrDesc);

vimStatus VIMAPIENTRY VIMSetCurrentSubtree(    vimAddressBook addressBook,
                                               vimRef subtreeRef,
                                               vimStringPtr subtreeName);

vimStatus VIMAPIENTRY VIMSetMessageHeader( vimMsg message,
                                           vimSelector selAttr,
                                           vimWord wAttrSize,
                                           vimDataPtr pAttr);

vimStatus VIMAPIENTRY VIMSetMessageItem(   vimMsg message,
                                           vimSelector selClass,
                                           vimStringPtr strType,
                                           vimSelector selFlags,
                                           vimStringPtr strName,
                                           vimBuffFileDescPtr pDesc );

vimStatus VIMAPIENTRY VIMSetMessageRecipient(  vimMsg message,
                                               vimSelector selClass,
                                               vimRecipientPtr pRecip);

vimStatus VIMAPIENTRY VIMStatusText(   vimSes session,
                                       vimStatus status,
                                       vimWord wTextSize,
                                       vimStringPtr pText,
                                       vimWord wExtTextSize,
                                       vimStringPtr pExtText);

vimStatus VIMAPIENTRY VIMTerminate( vimVoid );

vimStatus VIMAPIENTRY VIMVerifyMessageSignature(   vimMsg message,
                                                   vimWord attrCount,
                                                   vimAttrDescPtr pAttrDesc);

/*
   Include local implementation extensions.
*/
#include "vimext.h"

#ifdef __cplusplus
}
#endif

#endif     /* end of #IFNDEF __VIM_INCLUDED */

/* EOF: vim.h */

#ifdef __cplusplus
}
#endif

