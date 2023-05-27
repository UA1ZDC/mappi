#pragma once

#include <qstring.h>
#include <security/pam_appl.h>


namespace meteo {

namespace auth {

class PamContext
{
public :
  static const char* SERVICE_NAME;

public :
  PamContext(const QString& username, const QString& password);
  ~PamContext();

  bool create();
  bool authenticate();
  bool terminate();

private :
  static int conversation(int num_msg, const struct pam_message** msg, struct pam_response** resp, void* appdata_ptr);

private :
  pam_handle_t* pam_handle_;
  QString username_;
  QString password_;
};

}

}
