#include <stdlib.h>
#include <iostream>
#include <boost/throw_exception.hpp>

#ifdef BOOST_NO_EXCEPTIONS
void
boost::throw_exception (const std::exception&)
{
  std::cerr << "Unhandled exception" << std::endl; // FIXME: do better
  abort ();
}
#endif
