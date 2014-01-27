#ifndef INTERVAL_HXX
# define INTERVAL_HXX

# include <algorithm>
# include <cassert>
# include "interval.hh"

  /**************/
 /*  Interval  */
/**************/

template <typename T>
EmptyInterval<T>* Interval<T>::create() noexcept
{
    return new EmptyInterval<T>();
}

template <typename T>
SolidInterval<T>* Interval<T>::create(T min, T max) throw (IntervalException)
{
    return new SolidInterval<T>{min, max};
}


  /*********************/
 /*  DefinedInterval  */
/*********************/

template <typename T>
T DefinedInterval<T>::getMin() const noexcept
{
    return min_;
}

template <typename T>
T DefinedInterval<T>::getMax() const noexcept
{
    return max_;
}

template <typename T>
DefinedInterval<T>::DefinedInterval(T min, T max) throw(IntervalException)
    : min_{min}, max_{max}
{
    if (min > max) throw IntervalException();
}

  /*******************/
 /*  EmptyInterval  */
/*******************/

template <typename T>
bool EmptyInterval<T>::supersetOf(const Interval<T>*) const noexcept
{
    return false;
}

template <typename T>
bool EmptyInterval<T>::subsetOf(const Interval<T>*) const noexcept
{
    return true;
}

template <typename T>
Interval<T>* EmptyInterval<T>::aggregate(const Interval<T>* o) const noexcept
{
    return o->clone();
}

template <typename T>
EmptyInterval<T>* EmptyInterval<T>::clone() const
{
    return new EmptyInterval<T>{};
}

  /*******************/
 /*  SolidInterval  */
/*******************/

template <typename T>
SolidInterval<T>::SolidInterval(T min, T max) throw(IntervalException)
    : DefinedInterval<T>{min, max}
{
}

template <typename T>
bool SolidInterval<T>::contains(T v) const noexcept
{
    return v >= this->min_ && v <= this->max_;
}

template <typename T>
bool SolidInterval<T>::consecutiveTo(const SolidInterval<T>* o) const noexcept
{
    return this->max_ == o->min_ || this->min_ == o->max_;
}

template <typename T>
bool SolidInterval<T>::overlaps(const SolidInterval<T>* o) const noexcept
{
    return o->max_ >= this->min_ && o->min_ <= this->max_;
}

template <typename T>
bool SolidInterval<T>::overlaps(const SolidInterval<T>& o) const noexcept
{
    return o.max_ >= this->min_ && o.min_ <= this->max_;
}

template <typename T>
bool operator<(const SolidInterval<T>& a, const SolidInterval<T>& b) noexcept
{
    return a.getMax() < b.getMin();
}

template <typename T>
bool operator>(const SolidInterval<T>& a, const SolidInterval<T>& b) noexcept
{
    return a.getMin() > b.getMax();
}

template <typename T>
bool operator==(const SolidInterval<T>& a, const SolidInterval<T>& b) noexcept
{
    return (a.getMin() == b.getMin()) && (a.getMax() == b.getMax());
}

template <typename T>
bool operator!=(const SolidInterval<T>& a, const SolidInterval<T>& b) noexcept
{
    return !(a == b);
}

template <typename T>
bool SolidInterval<T>::supersetOf(const Interval<T>* o) const noexcept
{
    if (o->toEmptyInterval()) return true;
    else return this->min_ <= o->toDefinedInterval()->getMin()
        && this->max_ >= o->toDefinedInterval()->getMax();
}

template <typename T>
bool SolidInterval<T>::subsetOf(const Interval<T>* o) const noexcept
{
    if (o->toEmptyInterval()) return false;
    if (o->toSolidInterval())
        return o->toSolidInterval()->min_ <= this->min_
            && o->toSolidInterval()->max_ >= this->max_;
    if (o->toFragmentedInterval()) return o->supersetOf(this);
    assert(false && "not reached");
    return false;
}

template <typename T>
Interval<T>* SolidInterval<T>::aggregate(const Interval<T>* o) const noexcept
{
    if (o->toEmptyInterval()) return this->clone();

    if (o->toSolidInterval())
    {
        if (o->toSolidInterval()->getMin() > this->max_
            || o->toSolidInterval()->getMax() < this->min_)
        {
            const FragmentedInterval<T>* a = new FragmentedInterval<T>{*this};
            Interval<T>* b = a->aggregate(o);
            delete a;
            return b;
        }
        else
        {
            return new SolidInterval<T>{
                std::min(o->toSolidInterval()->getMin(), this->min_),
                std::max(o->toSolidInterval()->getMax(), this->max_)
            };
        }
    }

    if (o->toFragmentedInterval())
    {
        return o->aggregate(this);
    }

    assert(false && "not reached");
    return new EmptyInterval<T>{};
}

template <typename T>
SolidInterval<T>* SolidInterval<T>::aggregateSolid(const SolidInterval<T>* o) const
throw(IntervalException)
{
    if (o->getMin() > this->max_ || o->getMax() < this->min_)
    {
        throw IntervalException{};
    }

    return new SolidInterval<T>{
        std::min(o->getMin(), this->min_),
        std::max(o->getMax(), this->max_)
    };
}

template <typename T>
SolidInterval<T>* SolidInterval<T>::clone() const
{
    return new SolidInterval<T>{*this};
}


  /************************/
 /*  FragmentedInterval  */
/************************/

template <typename T>
FragmentedInterval<T>::FragmentedInterval(const SolidInterval<T>& solid)
    : DefinedInterval<T>{solid.getMin(), solid.getMax()}
{
    fragments_.push_back(solid);
}

template <typename T>
FragmentedInterval<T>::FragmentedInterval(SolidInterval<T>&& solid)
    : DefinedInterval<T>{solid.getMin(), solid.getMax()}
{
    fragments_.emplace_back(std::move(solid));
}

template <typename T>
bool FragmentedInterval<T>::contains(T v) const noexcept
{
    for (const SolidInterval<T>& s: fragments_)
    {
        if (s.contains(v)) return true;
    }
    return false;
}

template <typename T>
bool FragmentedInterval<T>::continuous() const noexcept
{
    auto it = fragments_.cbegin();
    T last = it->getMax();
    for (++it; it != fragments_.cend(); ++it)
    {
        if (last == it->getMin())
        {
            last = it->getMax();
        }
        else
        {
            return false;
        }
    }
    return true;
}

template <typename T>
bool FragmentedInterval<T>::supersetOf(const Interval<T>* o) const noexcept
{
    if (o->toEmptyInterval()) return true;

    if (o->toSolidInterval())
    {
        const SolidInterval<T>* so = o->toSolidInterval();
        return std::any_of(fragments_.cbegin(), fragments_.cend(),
                           [so](const SolidInterval<T>& s){ return s.supersetOf(so); });
    }

    if (o->toFragmentedInterval())
    {
        for (const SolidInterval<T>& s: o->toFragmentedInterval()->fragments_)
        {
            if (!this->supersetOf(&s)) return false;
        }
        return true;
    }

    assert(false && "not reached");
    return false;
}

template <typename T>
bool FragmentedInterval<T>::subsetOf(const Interval<T>* o) const noexcept
{
    if (o->toEmptyInterval()) return false;

    if (o->toSolidInterval())
    {
        return o->toSolidInterval()->getMin() <= this->min_
            && o->toSolidInterval()->getMax() >= this->max_;
    }

    if (o->toFragmentedInterval())
    {
        return o->supersetOf(this);
    }

    assert(false && "not reached");
    return false;
}

template <typename T>
Interval<T>* FragmentedInterval<T>::aggregate(const Interval<T>* o) const noexcept
{
    if (o->toEmptyInterval()) return this->clone();

    if (o->toSolidInterval())
    {
        auto newf = new FragmentedInterval<T>{*this};
        for (const SolidInterval<T>& s: fragments_)
        {
            if (s < *(o->toSolidInterval())) continue;

            auto it = std::find(newf->fragments_.begin(), newf->fragments_.end(), s);
            assert(it != newf->fragments_.end());
            if (s.overlaps(o->toSolidInterval()))
            {
                *it = *(it->aggregateSolid(o->toSolidInterval()));
            }
            else
            {
                newf->fragments_.insert(it, *(o->toSolidInterval()));
            }

            newf->min_ = std::min(this->min_, o->toSolidInterval()->getMin());
            newf->max_ = std::max(this->max_, o->toSolidInterval()->getMax());

            return newf;
        }
        newf->fragments_.push_back(*(o->toSolidInterval()));
        return newf;
    }

    if (o->toFragmentedInterval())
    {
        auto a = fragments_.cbegin();
        auto aend = fragments_.cend();
        auto b = o->toFragmentedInterval()->fragments_.cbegin();
        auto bend = o->toFragmentedInterval()->fragments_.cend();

        auto newf = new FragmentedInterval<T>;

        while (a != aend && b != bend)
        {
            if (*a < *b)
            {
                newf->fragments_.push_back(*a++);
            }
            else if (*b < *a)
            {
                newf->fragments_.push_back(*b++);
            }
            else
            {
                auto temp = new SolidInterval<T>{*a};
                do
                {
                    if (*temp != *a && temp->overlaps(*a)) {
                        SolidInterval<T>* ttemp = temp->aggregateSolid(&(*a++));
                        delete temp;
                        temp = ttemp;
                    }
                    if (temp->overlaps(*b)) {
                        SolidInterval<T>* ttemp = temp->aggregateSolid(&(*b++));
                        delete temp;
                        temp = ttemp;
                    }
                }
                while (temp->overlaps(*a) || temp->overlaps(*b));
                newf->fragments_.push_back(*temp);
            }
        }

        newf->min_ = std::min(this->min_, o->toFragmentedInterval()->min_);
        newf->max_ = std::max(this->max_, o->toFragmentedInterval()->max_);

        return newf;
    }

    assert(false && "not reached");
    return new EmptyInterval<T>{};
}

template <typename T>
FragmentedInterval<T>* FragmentedInterval<T>::clone() const
{
    return new FragmentedInterval<T>{*this};
}

#endif //!INTERVAL_HXX
