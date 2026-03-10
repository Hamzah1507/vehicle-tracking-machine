/*
 * ============================================================
 *  gps_utils.h  —  GPS Parsing & Location Utilities
 *  Vehicle Tracking Machine
 *  Author: Hamzah Saiyed
 * ============================================================
 */

#ifndef GPS_UTILS_H
#define GPS_UTILS_H

#include <Arduino.h>
#include <TinyGPS++.h>
#include <math.h>

#define EARTH_RADIUS_KM 6371.0

// ─────────────────────────────────────────────
//  GPS Data Packet
// ─────────────────────────────────────────────
struct GPSPacket {
  double   latitude;
  double   longitude;
  double   altitude_m;
  double   speed_kmph;
  double   course_deg;
  uint32_t satellites;
  double   hdop;           // Horizontal dilution of precision
  bool     isValid;
  uint16_t year;
  uint8_t  month, day;
  uint8_t  hour, minute, second;
};

// ─────────────────────────────────────────────
//  GPS Utility Class
// ─────────────────────────────────────────────
class GPSUtils {
  public:

    // ── Parse TinyGPS++ object into packet ───
    static GPSPacket parse(TinyGPSPlus& gps) {
      GPSPacket pkt = {0};

      pkt.isValid = gps.location.isValid();
      if (!pkt.isValid) return pkt;

      pkt.latitude    = gps.location.lat();
      pkt.longitude   = gps.location.lng();
      pkt.altitude_m  = gps.altitude.meters();
      pkt.speed_kmph  = gps.speed.kmph();
      pkt.course_deg  = gps.course.deg();
      pkt.satellites  = gps.satellites.value();
      pkt.hdop        = gps.hdop.hdop();

      if (gps.date.isValid()) {
        pkt.year  = gps.date.year();
        pkt.month = gps.date.month();
        pkt.day   = gps.date.day();
      }
      if (gps.time.isValid()) {
        pkt.hour   = gps.time.hour();
        pkt.minute = gps.time.minute();
        pkt.second = gps.time.second();
      }

      return pkt;
    }

    // ── Haversine distance (km) ───────────────
    static double haversineDistance(double lat1, double lon1,
                                    double lat2, double lon2) {
      double dLat = toRad(lat2 - lat1);
      double dLon = toRad(lon2 - lon1);

      double a = sin(dLat / 2) * sin(dLat / 2) +
                 cos(toRad(lat1)) * cos(toRad(lat2)) *
                 sin(dLon / 2) * sin(dLon / 2);
      double c = 2 * atan2(sqrt(a), sqrt(1 - a));
      return EARTH_RADIUS_KM * c;
    }

    // ── Bearing from point A → B (degrees) ───
    static double bearing(double lat1, double lon1,
                          double lat2, double lon2) {
      double dLon = toRad(lon2 - lon1);
      double y    = sin(dLon) * cos(toRad(lat2));
      double x    = cos(toRad(lat1)) * sin(toRad(lat2)) -
                    sin(toRad(lat1)) * cos(toRad(lat2)) * cos(dLon);
      double brng = toDeg(atan2(y, x));
      return fmod((brng + 360.0), 360.0);
    }

    // ── Cardinal direction from degrees ──────
    static String cardinalDirection(double degrees) {
      const char* dirs[] = {
        "N","NNE","NE","ENE","E","ESE","SE","SSE",
        "S","SSW","SW","WSW","W","WNW","NW","NNW"
      };
      int idx = (int)((degrees + 11.25) / 22.5) % 16;
      return String(dirs[idx]);
    }

    // ── Print packet to Serial ────────────────
    static void printPacket(const GPSPacket& pkt) {
      if (!pkt.isValid) {
        Serial.println("[GPS] No fix yet...");
        return;
      }
      Serial.printf("[GPS] %.6f, %.6f | Alt: %.1fm | Speed: %.2fkm/h | "
                    "Course: %.1f° (%s) | Sats: %d | HDOP: %.1f\n",
                    pkt.latitude, pkt.longitude,
                    pkt.altitude_m, pkt.speed_kmph,
                    pkt.course_deg, cardinalDirection(pkt.course_deg).c_str(),
                    pkt.satellites, pkt.hdop);
    }

    // ── Check signal quality ──────────────────
    static String signalQuality(double hdop) {
      if      (hdop <= 1.0) return "IDEAL";
      else if (hdop <= 2.0) return "EXCELLENT";
      else if (hdop <= 5.0) return "GOOD";
      else if (hdop <= 10.) return "MODERATE";
      else if (hdop <= 20.) return "FAIR";
      else                  return "POOR";
    }

  private:
    static double toRad(double deg) { return deg * M_PI / 180.0; }
    static double toDeg(double rad) { return rad * 180.0 / M_PI; }
};

#endif // GPS_UTILS_H