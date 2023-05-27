#include "pam_context.h"
#include <cross-commons/debug/tlog.h>


namespace meteo {

namespace auth {

const char* PamContext::SERVICE_NAME = "common-auth";

PamContext::PamContext(const QString& username, const QString& password) :
  pam_handle_(nullptr),
  username_(username),
  password_(password)
{
}

PamContext::~PamContext()
{
  terminate();
}

bool PamContext::create()
{
  const struct pam_conv conv = { conversation, this };
  return (pam_start(SERVICE_NAME, username_.toStdString().c_str(), &conv, &pam_handle_) == PAM_SUCCESS);
}

bool PamContext::authenticate()
{
  return (pam_authenticate(pam_handle_, 0) == PAM_SUCCESS);
}

bool PamContext::terminate()
{
  if (pam_end(pam_handle_, 0) == PAM_SUCCESS) {
    pam_handle_ = nullptr;
    return true;
  }

  return false;
}

int PamContext::conversation(int num_msg, const struct pam_message** msg, struct pam_response** resp, void* appdata_ptr)
{
  bool isOk = true;
  if (num_msg <= 0 || num_msg > PAM_MAX_NUM_MSG) {
    error_log << QObject::tr("[auth:pam] некорректный номер сообщения: %1").arg(num_msg);
    return PAM_CONV_ERR;
  }

  *resp = new pam_response[num_msg];

  for (int i = 0; i < num_msg; ++i) {
    const pam_message* msg_item = msg[i];
    if (msg_item == nullptr) {
      isOk = false;
      error_log << QObject::tr("[auth:pam] невалидное сообщение: %1, %2")
                     .arg(i)
                     .arg(msg_item->msg_style);
      break ;
    }

    pam_response* resp_item = resp[i];
    resp_item->resp_retcode = 0;
    resp_item->resp = nullptr;

    switch (msg_item->msg_style) {
      case PAM_PROMPT_ECHO_ON :
      case PAM_PROMPT_ECHO_OFF : {
          PamContext* app = static_cast<PamContext*>(appdata_ptr);
          resp_item->resp = strdup(app->password_.toStdString().c_str());
        }
        break ;

      case PAM_ERROR_MSG :
        error_log << QObject::tr("[auth:pam] %1").arg(msg_item->msg);
        break ;

      case PAM_TEXT_INFO :
        debug_log << QObject::tr("[auth:pam] %1").arg(msg_item->msg);
        break ;

      default :
        isOk = false;
        break ;
    }

    if (!isOk)
      break ;
  }

  if (!isOk) {
    delete[] *resp;
    *resp = nullptr;

    return PAM_CONV_ERR;
  }

  return PAM_SUCCESS;
}

}

}