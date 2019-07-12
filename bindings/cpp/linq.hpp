#ifndef LINQ_HPP_
#define LINQ_HPP_

#include <string>

#include "altroinx/linq.h"

namespace altronix {

class Linq
{
  public:
    Linq()
        : Linq(NULL, NULL){};
    Linq(linq_callbacks* callbacks, void* context)
    {
        linq_ = linq_create(callbacks, context);
    }
    ~Linq() { linq_destroy(&linq_); }

  private:
    linq* linq_;
};

} // namespace altronix

#endif /* LINQ_HPP_ */
