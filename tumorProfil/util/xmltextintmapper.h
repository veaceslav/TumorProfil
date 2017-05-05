/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 25.02.2013
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef XMLTEXTINTMAPPER_H
#define XMLTEXTINTMAPPER_H

/**
  Helper to facilitate Enum-To-String and String-To-Enum mapping.
  Assume we have
  class Foo
  {
    enum Bar
    {
        A, B, C
    };
  };
  Syntax:
  TEXT_INT_MAPPER(Foo, Bar)
  {
    Pair("A", A),
    Pair("B", B),
    Pair("C", C)
  }

  In code, to translate enum to string:
  FooBarTextIntMapper::toString(value)
  To translate string to enum:
  FooBarTextIntMapper::toEnum(s)
  */

template <typename Enum, class Child>
class TextIntMapper
{
public:

    TextIntMapper(Enum enu)
        : enu(enu), text(toString(enu))
    {}
    TextIntMapper(const QString& s)
        : enu(toEnum(s)), text(s)
    {}
    operator QStringRef() const { return text; }
    operator Enum() const { return enu; }

    Enum       enu;
    QString    text;

    static QPair<QLatin1String,Enum> Pair(const char* s, Enum i)
    { return qMakePair(QLatin1String(s), i); }

    static QString toString(Enum c)
    {
        for (uint i=0; i<sizeof(Child::map)/sizeof(QPair<QLatin1String,Enum>); i++)
        {
            if (Child::map[i].second == c)
            {
                return Child::map[i].first;
            }
        }
        return QString();
    }

    template <class S> // with S = QString or QStringRef
    static Enum toEnum(const S& s)
    {
        for (uint i=0; i<sizeof(Child::map)/sizeof(QPair<QLatin1String,Enum>); i++)
        {
            if (Child::map[i].first == s)
            {
                return Child::map[i].second;
            }
        }
        return static_cast<Enum>(0);
    }
};

#define TEXT_INT_MAPPER(Class, Enum) \
    class Class##Enum##TextIntMapper : public TextIntMapper<Class::Enum, Class##Enum##TextIntMapper> \
{ public: \
static const QPair<QLatin1String,Class::Enum> map[]; \
}; \
const QPair<QLatin1String,Class::Enum> Class##Enum##TextIntMapper::map[] =

#define EVENT_TEXT_INT_MAPPER(Class, Enum) \
    class Event##Class##Enum##TextIntMapper : public TextIntMapper<Class::Enum, Event##Class##Enum##TextIntMapper> \
{ public: \
Event##Class##Enum##TextIntMapper(Class::Enum e) : TextIntMapper<Class::Enum, Event##Class##Enum##TextIntMapper>(e) {} \
Event##Class##Enum##TextIntMapper(const QString& s) : TextIntMapper<Class::Enum, Event##Class##Enum##TextIntMapper>(s) {} \
static const QPair<QLatin1String,Class::Enum> map[]; \
}; \
const QPair<QLatin1String,Class::Enum> Event##Class##Enum##TextIntMapper::map[] =



#endif // XMLTEXTINTMAPPER_H
