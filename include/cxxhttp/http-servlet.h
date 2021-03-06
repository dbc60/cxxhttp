/* HTTP servlet.
 *
 * This contains the servlet type used by the default HTTP server processor.
 *
 * This file implements an actual HTTP server with a simple servlet capability
 * in top of http.h.
 *
 * See also:
 * * Project Documentation: https://ef.gy/documentation/cxxhttp
 * * Project Source Code: https://github.com/ef-gy/cxxhttp
 * * Licence Terms: https://github.com/ef-gy/cxxhttp/blob/master/COPYING
 *
 * @copyright
 * This file is part of the cxxhttp project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 */
#if !defined(CXXHTTP_HTTP_SERVLET_H)
#define CXXHTTP_HTTP_SERVLET_H

#include <regex>

#include <ef.gy/global.h>

#include <cxxhttp/http-header.h>
#include <cxxhttp/http-session.h>

namespace cxxhttp {
namespace http {
/* HTTP servlet container.
 *
 * This contains all the data needed to set up a subprocessor for the default
 * server processor type. This consists of regexen to match against a requested
 * resource and the method invoked, as well as a handler for a succsseful match.
 *
 * For advanced usage, it is possible to provide data or content negotiation.
 */
class servlet {
 public:
  /* Constructor.
   * @pResourcex Regex for applicable resources.
   * @pHandler Function specification to handle incoming requests that match.
   * @pMethodx Optional method regex; defaults to only allowing GET.
   * @pNegotiations Map of content negotiations to perform for this servlet.
   * @pDescription Optional API description string. URL recommended.
   * @pSet Where to register the servlet; defaults to the global set.
   *
   * Initialisation allows setting all the available options. The only reason
   * this isn't a POD type is that we also need to register with the
   * per-transport set of servlets.
   */
  servlet(const std::string &pResourcex,
          std::function<void(sessionData &, std::smatch &)> pHandler,
          const std::string &pMethodx = "GET",
          const http::headers pNegotiations = {},
          const std::string &pDescription = "no description available",
          efgy::beacons<servlet> &pSet = efgy::global<efgy::beacons<servlet>>())
      : resourcex(pResourcex),
        resource(pResourcex),
        methodx(pMethodx),
        method(pMethodx),
        negotiations(pNegotiations),
        handler(pHandler),
        description(pDescription),
        beacon(*this, pSet) {}

  /* Resource regex.
   *
   * A regex that is matched, in full, against any incoming requests. The
   * matches structure that results is passed into the handler function, so it
   * is recommended that interesting parts of the URL are captured using
   * parentheses to indicate subexpressions.
   */
  const std::string resourcex;

  /* Compiled resource regex.
   *
   * The compiled form of <resourcex>.
   */
  const std::regex resource;

  /* Method regex.
   *
   * Similar to the resource regex, but for the method that is used by the
   * client. The matches struct here is not provided to the handler, but the
   * original method name is provided as part of the session object.
   */
  const std::string methodx;

  /* Compiled method regex.
   *
   * The compiled version of <methodx>.
   */
  const std::regex method;

  /* Content negotiation data.
   *
   * This is a map of the form `header: valid options`. Any header specified
   * will be automatically negotiated against what the client specifies during
   * the request, while allowing q-values on either side.
   *
   * If there's no matches between what both sides provide, then this is
   * considered a bad match, and a reply is sent to the client indicating that
   * content negotiation has failed.
   *
   * As an example, suppose this is set to:
   *
   *     {
   *       {"Accept": "text/plain, application/json;q=0.9"}
   *     }
   *
   * If the client sent no `Accept` header, the negotiated value would be
   * `text/plain`. If it sent the value `application/json` then this would pick
   * `application/json`. If it sent `foo/blargh` then that would be a bad match,
   * resulting in a client error unless a different servlet that is applicable
   * matches successfully.
   *
   * Also note that `Accept` in particular will cause an appropriate
   * `Content-Type` header with the correct negotiation to be sent back
   * automatically. This is configured in one of the constants in
   * `http-constants.h`. Also, each match will add the original header to the
   * `Vary` header that will also be added to the output automatically.
   */
  const http::headers negotiations;

  /* Handler function.
   *
   * Will be invoked if the resource and method match the provided regexen, and
   * content negotiation was successful.
   *
   * The return type is a boolean, and returning `true` indicates that the
   * request has been adequately processed. If this function returns `false`,
   * then the processor will continue trying to look up matching handlers and
   * assume no reply has been sent, yet.
   */
  const std::function<void(sessionData &, std::smatch &)> handler;

  /* Description of the servlet.
   *
   * Help texts may use this to provide more details on what a servlet does and
   * how to invoke it correctly. A URL with an API description si recommended.
   */
  const std::string description;

  /* Generate a description of the servlets.
   *
   * Creates a Markdown snippet using the method and resource regexen, along
   * with the description string. This is used by the default implementation of
   * the OPTIONS method or the CLI usage hint.
   *
   * @return A Markdown snippet describing the servlet.
   */
  std::string describe(void) const {
    return " * _" + methodx + "_ `" + resourcex + "`\n" + "   " + description +
           "\n\n";
  }

 protected:
  /* Servlet beacon.
   *
   * We need to keep track of all servlets in a central place, so that the
   * processing code knows to apply them where necessary.
   */
  efgy::beacon<servlet> beacon;
};
}
}

#endif
