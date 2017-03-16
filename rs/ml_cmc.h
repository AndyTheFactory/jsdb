/****************************************************************/
/*                                                              */
/*  CMFUNC. H                                                   */
/*                                                              */
/****************************************************************/

#ifndef _RS_ML_CMC_H_
#define _RS_ML_CMC_H_ 

/* Default filename for CMC DLL */
#define CMCDLL       "CMC.DLL"

/* CMC function pointer datatypes */

/* cmc_send */
typedef CMC_return_code (CMC_CALLBACK PCMCSEND)
             (CMC_session_id     session,
              CMC_message        *message,
              CMC_flags          send_flags,
              CMC_ui_id          ui_id,
              CMC_extension      *send_extensions);

/* cmc_send_documents */
typedef CMC_return_code (CMC_CALLBACK PCMCSENDDOCUMENTS)
             (CMC_string         recipient_addresses,
              CMC_string         subject,
              CMC_string         text_note,
              CMC_flags          send_doc_flags,
              CMC_string         file_paths,
              CMC_string         attach_titles,
              CMC_string         delimiter,
              CMC_ui_id          ui_id);

/* cmc_act_on */
typedef CMC_return_code (CMC_CALLBACK PCMCACTON)
             (CMC_session_id     session,
              CMC_message_reference  *message_reference,
              CMC_enum           operation,
              CMC_flags          act_on_flags,
              CMC_ui_id          ui_id,
              CMC_extension      *act_on_extensions);

/* cmc_list */
typedef CMC_return_code (CMC_CALLBACK PCMCLIST)
             (CMC_session_id     session,
              CMC_string         message_type,
              CMC_flags          list_flags,
              CMC_message_reference  *seed,
              CMC_uint32         *count,
              CMC_ui_id          ui_id,
              CMC_message_summary    **result,
              CMC_extension      *list_extensions);

/* cmc_read */
typedef CMC_return_code (CMC_CALLBACK PCMCREAD)
             (CMC_session_id     session,
              CMC_message_reference  *message_reference,
              CMC_flags          read_flags,
              CMC_message        **message,
              CMC_ui_id          ui_id,
              CMC_extension      *read_extensions);

/* cmc_look_up */
typedef CMC_return_code (CMC_CALLBACK PCMCLOOKUP)
             (CMC_session_id     session,
              CMC_recipient      *recipient_in,
              CMC_flags          look_up_flags,
              CMC_ui_id          ui_id,
              CMC_uint32         *count,
              CMC_recipient      **recipient_out,
              CMC_extension      *look_up_extensions);

/* cmc_free */
typedef CMC_return_code (CMC_CALLBACK PCMCFREE)
             (CMC_buffer         memory);

/* cmc_logoff */
typedef CMC_return_code (CMC_CALLBACK PCMCLOGOFF)
             (CMC_session_id     session,
              CMC_ui_id          ui_id,
              CMC_flags          logoff_flags,
              CMC_extension      *logoff_extensions);

/* cmc_logon */
typedef CMC_return_code (CMC_CALLBACK PCMCLOGON)
             (CMC_string         service,
              CMC_string         user,
              CMC_string         password,
              CMC_object_identifier  character_set,
              CMC_ui_id          ui_id,
              CMC_uint16         caller_cmc_version,
              CMC_flags          logon_flags,
              CMC_session_id     *session,
              CMC_extension      *logon_extensions);

/* cmc_query_configuration */
typedef CMC_return_code (CMC_CALLBACK PCMCQUERYCONFIG)
             (CMC_session_id     session,
              CMC_enum           item,
              CMC_buffer         reference,
              CMC_extension      *config_extensions);
#endif /* __CMCFUNC_H_ */

