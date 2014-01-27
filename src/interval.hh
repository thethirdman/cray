#ifndef INTERVAL_HH
# define INTERVAL_HH

# include <stdexcept>
# include <list>

class IntervalException: public std::exception {};

template <typename T> class EmptyInterval;
template <typename T> class DefinedInterval;
template <typename T> class FragmentedInterval;
template <typename T> class SolidInterval;

  /**************/
 /*  Interval  */
/**************/

template <typename T>
class Interval
{
public:
    static EmptyInterval<T>* create() noexcept;
    static SolidInterval<T>* create(T min, T max) throw(IntervalException);

    virtual ~Interval() {};

    // Is v in this interval?
    virtual bool contains(T v) const noexcept = 0;

    // Does this interval contain o entirely (inclusive)?
    virtual bool supersetOf(const Interval<T>* o) const noexcept = 0;

    // Does o contain this interval entirely (inclusive)?
    virtual bool subsetOf(const Interval<T>* o) const noexcept = 0;

    virtual Interval<T>* aggregate(const Interval<T>* o) const noexcept = 0;

    // Safe casts
    virtual EmptyInterval<T>*
        toEmptyInterval() const noexcept { return nullptr; }
    virtual DefinedInterval<T>*
        toDefinedInterval() const noexcept { return nullptr; }
    virtual FragmentedInterval<T>*
        toFragmentedInterval() const noexcept { return nullptr; }
    virtual SolidInterval<T>*
        toSolidInterval() const noexcept { return nullptr; }

    virtual Interval<T>* clone() const = 0;
};


template <typename T>
class DefinedInterval: public Interval<T>
{
public:
    virtual ~DefinedInterval() {};

    inline T    getMin() const noexcept;
    inline T    getMax() const noexcept;

    // Is this interval continuous, i.e. without holes?
    virtual bool continuous() const noexcept = 0;

    DefinedInterval<T>*
        toDefinedInterval() const noexcept override {
            return const_cast<DefinedInterval<T>*>(this);
        }

protected:
    explicit DefinedInterval() = default;
    inline DefinedInterval(T min, T max) throw(IntervalException);
    T min_;
    T max_;
};


  /*******************/
 /*  EmptyInterval  */
/*******************/

template <typename T>
class EmptyInterval: public Interval<T>
{
public:
    EmptyInterval() = default;

    // Is v in this interval?
    bool contains(T) const noexcept { return false; };

    // Does this interval contain o entirely (inclusive)?
    inline bool supersetOf(const Interval<T>* o) const noexcept override;

    // Does o contain this interval entirely (inclusive)?
    bool subsetOf(const Interval<T>*) const noexcept override;

    Interval<T>* aggregate(const Interval<T>* o) const noexcept override;

    EmptyInterval<T>*
        toEmptyInterval() const noexcept override {
            return const_cast<EmptyInterval<T>*>(this);
        }

    EmptyInterval<T>* clone() const override;
};


  /*******************/
 /*  SolidInterval  */
/*******************/

template <typename T>
class SolidInterval: public DefinedInterval<T>
{
public:
    inline      SolidInterval(T min, T max) throw(IntervalException);

    // Is v in this interval?
    inline bool contains(T v) const noexcept override;

    // Is this interval continuous, i.e. without holes?
    bool continuous() const noexcept override { return true; };

    // Consecutive borders
    inline bool consecutiveTo(const SolidInterval<T>* o) const noexcept;

    // Non-empty intersection
    inline bool overlaps(const SolidInterval<T>* o) const noexcept;
    inline bool overlaps(const SolidInterval<T>& o) const noexcept;

    // Does this interval contain o entirely (inclusive)?
    bool    supersetOf(const Interval<T>* o) const noexcept override;

    // Does o contain this interval entirely (inclusive)?
    bool    subsetOf(const Interval<T>* o) const noexcept override;

    // Union
    Interval<T>* aggregate(const Interval<T>* o) const noexcept override;
    SolidInterval<T>*
        aggregateSolid(const SolidInterval<T>* o) const throw(IntervalException);

    // Safe cast
    SolidInterval<T>*
        toSolidInterval() const noexcept override {
            return const_cast<SolidInterval<T>*>(this);
        }

    SolidInterval<T>* clone() const override;
};

template <typename T>
inline bool operator<(const SolidInterval<T>* a, const SolidInterval<T>* b) noexcept;

template <typename T>
inline bool operator>(const SolidInterval<T>* a, const SolidInterval<T>* b) noexcept;

template <typename T>
inline bool operator==(const SolidInterval<T>* a, const SolidInterval<T>* b) noexcept;

template <typename T>
inline bool operator!=(const SolidInterval<T>* a, const SolidInterval<T>* b) noexcept;


  /************************/
 /*  FragmentedInterval  */
/************************/

template <typename T>
class FragmentedInterval: public DefinedInterval<T>
{
    std::list<SolidInterval<T>>    fragments_;

public:
    FragmentedInterval(const SolidInterval<T>&  solid);
    FragmentedInterval(SolidInterval<T>&&       solid);

    // Is v in this interval?
    bool    contains(T v) const noexcept override;

    // Is this interval continuous, i.e. without holes?
    bool continuous() const noexcept override;

    // Does this interval contain o entirely (inclusive)?
    bool    supersetOf(const Interval<T>* o) const noexcept override;

    // Does o contain this interval entirely (inclusive)?
    bool    subsetOf(const Interval<T>* o) const noexcept override;

    // Union
    Interval<T>* aggregate(const Interval<T>* o) const noexcept override;

    // Safe cast
    FragmentedInterval<T>*
        toFragmentedInterval() const noexcept override {
            return const_cast<FragmentedInterval<T>*>(this);
        }

    FragmentedInterval<T>* clone() const override;

private:
    explicit FragmentedInterval() = default;
};

# include "interval.hxx"

#endif //!INTERVAL_HH
