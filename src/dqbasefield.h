#ifndef DQBASEFIELD_H
#define DQBASEFIELD_H

#include <QExplicitlySharedDataPointer>
#include <QVariant>
#include <dqclause.h>

class DQModel;

class DQBaseFieldPriv;

/// The base class of DQField
/**

  @remarks It is an explicity shared class
 */
class DQBaseField
{
public:
    DQBaseField();
    DQBaseField(const DQBaseField &rhs);

    ~DQBaseField();

    /// Assign value to the field
    virtual bool set(QVariant value);

    /// Get the value of the field
    /**
      @param convert True if the QVariant return should be converted to a type which is suitable for saving.

      Some data type like QStringList is not suitable for saving. User may override
      this function and convert its to other type.
     */
    virtual QVariant get(bool convert = false) const;

    /// The default clause of that field type
    static DQClause clause();

    /// Assign the value from a QVariant type source.
    virtual QVariant operator=(const QVariant &val);

    virtual DQBaseField &operator=(const DQBaseField &);

    /// Provides access to stored QVariant value
    QVariant* operator->();

    /// Get the value of the field
    QVariant operator() ()const;

    /// Cast to QVariant
    operator QVariant();

    /// Free up any resources used.
    void clear();

    /// Detach from the shared data object
    /**
      DQBaseField is an explicitly shared class. It hold a reference to a shared data object. If the object's reference count is greater than 1, this function
      creates a deep copy from the shared data object and unlink the reference. The result will be an independent object.
     */
    void detach();

private:
    QExplicitlySharedDataPointer<DQBaseFieldPriv> d;
};

QDebug operator<<(QDebug dbg, const DQBaseField &field);


#endif // DQBASEFIELD_H
