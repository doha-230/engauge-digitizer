/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef POINT_ORIGIN_H
#define POINT_ORIGIN_H

#include <QString>

/// How a digitized graph point came into existence.
///
/// This is recorded in the document so that points placed by automation can be told
/// apart from points placed by hand: an automated point can be discarded and
/// re-digitized after the color filter or the grid removal settings change, without
/// throwing away manual corrections.
///
/// POINT_ORIGIN_UNKNOWN serves two purposes: it is what older documents contain
/// (the attribute is optional, so nothing has to be migrated), and it is what the
/// serialized document stores whenever the origin is not known, so that documents
/// written by this build remain readable by builds that do not know the attribute.
enum PointOrigin {
  POINT_ORIGIN_UNKNOWN = 0,
  POINT_ORIGIN_MANUAL,       ///< Curve Point Tool: one point per mouse click
  POINT_ORIGIN_SEGMENT_FILL, ///< Segment Fill Tool: many points along a segment
  POINT_ORIGIN_POINT_MATCH,  ///< Point Match Tool: a point snapped onto a sample point
  POINT_ORIGIN_PASTED        ///< Points pasted from the clipboard or another document
};

extern const QString POINT_ORIGIN_VALUE_UNKNOWN;
extern const QString POINT_ORIGIN_VALUE_MANUAL;
extern const QString POINT_ORIGIN_VALUE_SEGMENT_FILL;
extern const QString POINT_ORIGIN_VALUE_POINT_MATCH;
extern const QString POINT_ORIGIN_VALUE_PASTED;

/// Serialized name for an origin, empty for POINT_ORIGIN_UNKNOWN
QString pointOriginToString (PointOrigin origin);

/// Origin for a serialized name. Unknown, missing and unparsable names all give POINT_ORIGIN_UNKNOWN
PointOrigin pointOriginFromString (const QString &name);

/// True when the origin means the point was created by automation rather than by hand
bool pointOriginIsAutomated (PointOrigin origin);

#endif // POINT_ORIGIN_H
