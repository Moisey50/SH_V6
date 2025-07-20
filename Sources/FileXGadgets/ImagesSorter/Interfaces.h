#pragma once

#define RESET_INSTANCE memset(this, 0, sizeof(*this));

template <typename T>
class IEquatable
{
public:
  virtual bool Equals(const T& other) const = 0;
};
