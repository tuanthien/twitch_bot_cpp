#pragma once
#include "Windows/PlatformInclude.hpp"

#include <boost/asio/ssl/context.hpp>

namespace TwitchBot {

auto AddRootCerts(boost::asio::ssl::context &ctx) -> bool
{
  HCERTSTORE hStore = CertOpenSystemStore(0, TEXT("ROOT"));
  if (hStore == NULL) {
    return false;
  }

  X509_STORE *store       = X509_STORE_new();
  PCCERT_CONTEXT pContext = NULL;
  while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != NULL) {
    X509 *x509 = d2i_X509(NULL, (const unsigned char **)&pContext->pbCertEncoded, pContext->cbCertEncoded);
    if (x509 != NULL) {
      X509_STORE_add_cert(store, x509);
      X509_free(x509);
    }
  }

  CertFreeCertificateContext(pContext);
  CertCloseStore(hStore, 0);

  SSL_CTX_set_cert_store(ctx.native_handle(), store);
  // @TODO memory leak
  return true;
}


}