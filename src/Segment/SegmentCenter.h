/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef SEGMENT_CENTER_H
#define SEGMENT_CENTER_H

#include <QList>
#include <QPoint>

class QImage;

/// Moves filled segment points to the center of the curve band, according to the selected strategy.
///
/// The upstream fill places points on the traced path, which can sit anywhere inside a thick band.
/// With a centering strategy enabled, every point is moved to the middle of the curve pixels in its
/// column, so the exported numbers follow the middle of the band instead of the traced edge.
///
/// With TRACED_PATH_CENTER this class changes nothing, which is the default and keeps the results
/// identical to the upstream release.
class SegmentCenter
{
public:
  /// Move every point to the band center according to the strategy. The image is the filtered image,
  /// where curve pixels are on. Returns the adjusted list; the input list is not modified.
  static QList<QPoint> centerPoints (const QList<QPoint> &points,
                                     const QImage &image,
                                     SegmentCenterStrategy strategy);

private:
  SegmentCenter ();

  /// Center of the run of on pixels that contains the specified y in the specified column, as a
  /// plain average of the pixel positions. Returns false when the pixel is off, or the run cannot
  /// be found.
  static bool pixelRunCenter (const QImage &image,
                              int x,
                              int y,
                              double &yCenter);

  /// Intensity weighted center of the run: darker pixels (stronger curve) count more. Returns false
  /// under the same conditions as pixelRunCenter.
  static bool weightedPixelRunCenter (const QImage &image,
                                      int x,
                                      int y,
                                      double &yCenter);

  /// Luminance of one pixel, 0 (black) to 255 (white)
  static int luminance (const QImage &image,
                        int x,
                        int y);
};

#endif // SEGMENT_CENTER_H
