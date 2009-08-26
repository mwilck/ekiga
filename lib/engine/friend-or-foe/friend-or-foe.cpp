#include "friend-or-foe.h"

Ekiga::FriendOrFoe::Identification
Ekiga::FriendOrFoe::decide (const std::string domain,
			    const std::string token) const
{
  Identification answer = Unknown;
  Identification iter_answer;

  for (helpers_type::const_iterator iter = helpers.begin ();
       iter != helpers.end ();
       ++iter) {

    iter_answer = (*iter)->decide (domain, token);
    if (answer < iter_answer)
      answer = iter_answer;
  }

  return answer;
}

void
Ekiga::FriendOrFoe::add_helper (boost::shared_ptr<Ekiga::FriendOrFoe::Helper> helper)
{
  helpers.push_front (helper);
}
