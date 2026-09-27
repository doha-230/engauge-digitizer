/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "SegmentCenter.h"
#include "Logger.h"
#include "SegmentCenterStrategy.h"
#include <QImage>
#include <qmath.h>

QList<QPoint> SegmentCenter::centerPoints (const QList<QPoint> &points,
                                           const QImage &image,
                                           SegmentCenterStrategy strategy)
{
  LOG4CPP_INFO_S ((*mainCat)) << "SegmentCenter::centerPoints"
                              << " count=" << points.count()
                              << " strategy=" << segmentCenterStrategyToString (strategy).toLatin1().data();

  if ((strategy == TRACED_PATH_CENTER) || points.isEmpty () || image.isNull ()) {
    // Upstream behavior: the traced path is kept as is
    return points;
  }

  QList<QPoint> result;

  for (int index = 0; index < points.count (); index++) {

    const QPoint point = points.at (index);
    const int x = point.x ();
    const int y = point.y ();

    double yCenter = y;
    bool moved = false;

    if (strategy == PIXEL_RUN_CENTER) {
      moved = pixelRunCenter (image, x, y, yCenter);
    } else if (strategy == WEIGHTED_PIXEL_CENTER) {
      moved = weightedPixelRunCenter (image, x, y, yCenter);
    }

    if (moved) {
      result << QPoint (x, qFloor (yCenter + 0.5));
    } else {
      // The pixel is off in the filtered image (for example a point between two bands), so the
      // traced position is the safest choice
      result << point;
    }
  }

  LOG4CPP_INFO_S ((*mainCat)) << "SegmentCenter::centerPoints moved="
                              << (strategy == TRACED_PATH_CENTER ? 0 : points.count ());

  return result;
}

int SegmentCenter::luminance (const QImage &image,
                              int x,
                              int y)
{
  if ((x < 0) || (image.width () <= x) || (y < 0) || (image.height () <= y)) {
    return 255; // Outside counts as background (white)
  }

  const QRgb rgb = image.pixel (x, y);

  return qGray (rgb);
}

bool SegmentCenter::pixelRunCenter (const QImage &image,
                                    int x,
                                    int y,
                                    double &yCenter)
{
  if ((x < 0) || (image.width () <= x) || (y < 0) || (image.height () <= y)) {
    return false;
  }

  // The filtered image marks curve pixels as on (dark in the QImage). Expand up and down from the
  // point until the band edges are found
  int yTop = y;
  while ((yTop > 0) && (qGray (image.pixel (x, yTop - 1)) < 128)) {
    yTop--;
  }

  int yBottom = y;
  while ((yBottom + 1 < image.height ()) && (qGray (image.pixel (x, yBottom + 1)) < 128)) {
    yBottom++;
  }

  yCenter = (yTop + yBottom) / 2.0;

  return true;
}

bool SegmentCenter::weightedPixelRunCenter (const QImage &image,
                                            int x,
                                            int y,
                                            double &yCenter)
{
  if ((x < 0) || (image.width () <= x) || (y < 0) || (image.height () <= y)) {
    return false;
  }

  // Expand to the band edges like pixelRunCenter does
  int yTop = y;
  while ((yTop > 0) && (qGray (image.pixel (x, yTop - 1)) < 128)) {
    yTop--;
  }

  int yBottom = y;
  while ((yBottom + 1 < image.height ()) && (qGray (image.pixel (x, yBottom + 1)) < 128)) {
    yBottom++;
  }

  // Weight every pixel of the band by how dark it is: the dark middle of a soft edged band counts
  // more than the bright fringe
  double weightSum = 0.0;
  double weightedYSum = 0.0;

  for (int yBand = yTop; yBand <= yBottom; yBand++) {
    const double weight = 255.0 - luminance (image, x, yBand);
    weightSum += weight;
    weightedYSum += weight * yBand;
  }

  if (weightSum <= 0.0) {
    return false;
  }

  yCenter = weightedYSum / weightSum;

  return true;
}
