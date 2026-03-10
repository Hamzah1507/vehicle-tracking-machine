/*
 * ============================================================
 *  server.js  —  Vehicle Tracking Backend
 *  Node.js + Express + Socket.IO + MongoDB
 *  Author: Hamzah Saiyed
 * ============================================================
 *
 *  Endpoints:
 *   POST /api/location        — Receive data from ESP32
 *   GET  /api/location/latest — Latest vehicle state
 *   GET  /api/location/history?limit=N — Historical track
 *   GET  /api/stats           — Summary statistics
 *   WS   socket.io            — Real-time push to dashboard
 * ============================================================
 */

'use strict';

const express    = require('express');
const http       = require('http');
const { Server } = require('socket.io');
const mongoose   = require('mongoose');
const cors       = require('cors');
const helmet     = require('helmet');
const morgan     = require('morgan');
const rateLimit  = require('express-rate-limit');
require('dotenv').config();

// ─────────────────────────────────────────────
//  App Setup
// ─────────────────────────────────────────────
const app    = express();
const server = http.createServer(app);
const io     = new Server(server, {
  cors: { origin: '*', methods: ['GET', 'POST'] }
});

const PORT      = process.env.PORT      || 3000;
const MONGO_URI = process.env.MONGO_URI || 'mongodb://localhost:27017/vehicle_tracker';

// ─────────────────────────────────────────────
//  Middleware
// ─────────────────────────────────────────────
app.use(cors());
app.use(helmet());
app.use(morgan('dev'));
app.use(express.json({ limit: '10kb' }));
app.use(express.static('frontend/public'));

// Rate limiter for ESP32 endpoint
const espLimiter = rateLimit({
  windowMs: 60 * 1000,
  max: 120,
  message: { error: 'Too many requests from this device' }
});

// ─────────────────────────────────────────────
//  MongoDB Schema
// ─────────────────────────────────────────────
const locationSchema = new mongoose.Schema({
  latitude:          { type: Number, required: true },
  longitude:         { type: Number, required: true },
  speed_kmph:        { type: Number, default: 0 },
  heading:           { type: Number, default: 0 },
  direction:         { type: String, enum: ['FORWARD','BACKWARD','LEFT','RIGHT','STOP'], default: 'STOP' },
  accel_x:           { type: Number, default: 0 },
  accel_y:           { type: Number, default: 0 },
  accel_z:           { type: Number, default: 0 },
  gyro_x:            { type: Number, default: 0 },
  gyro_y:            { type: Number, default: 0 },
  gyro_z:            { type: Number, default: 0 },
  distance_cm:       { type: Number, default: 0 },
  obstacle_detected: { type: Boolean, default: false },
  timestamp:         { type: Date, default: Date.now }
});

// Geospatial index for map queries
locationSchema.index({ timestamp: -1 });
locationSchema.index({ latitude: 1, longitude: 1 });

const Location = mongoose.model('Location', locationSchema);

// ─────────────────────────────────────────────
//  In-memory cache for latest state
// ─────────────────────────────────────────────
let latestVehicleState = null;
let totalPacketsReceived = 0;

// ─────────────────────────────────────────────
//  Routes
// ─────────────────────────────────────────────

// Health check
app.get('/health', (req, res) => {
  res.json({
    status  : 'OK',
    uptime  : process.uptime().toFixed(2) + 's',
    packets : totalPacketsReceived,
    db      : mongoose.connection.readyState === 1 ? 'connected' : 'disconnected'
  });
});

// POST /api/location — receive data from ESP32
app.post('/api/location', espLimiter, async (req, res) => {
  try {
    const {
      latitude, longitude, speed_kmph, heading, direction,
      accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z,
      distance_cm, obstacle_detected
    } = req.body;

    // Basic validation
    if (latitude  === undefined || longitude === undefined) {
      return res.status(400).json({ error: 'latitude and longitude are required' });
    }
    if (latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
      return res.status(400).json({ error: 'Invalid GPS coordinates' });
    }

    // Save to MongoDB
    const doc = new Location({
      latitude, longitude, speed_kmph, heading, direction,
      accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z,
      distance_cm, obstacle_detected,
      timestamp: new Date()
    });
    await doc.save();

    // Update cache
    latestVehicleState = doc.toObject();
    totalPacketsReceived++;

    // Broadcast to all connected dashboard clients
    io.emit('vehicleUpdate', latestVehicleState);

    console.log(`[API] Packet #${totalPacketsReceived} | Dir: ${direction} | Lat: ${latitude} Lon: ${longitude}`);
    res.status(200).json({ success: true, id: doc._id });

  } catch (err) {
    console.error('[API] Error saving location:', err.message);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// GET /api/location/latest
app.get('/api/location/latest', (req, res) => {
  if (!latestVehicleState) {
    return res.status(404).json({ error: 'No data yet' });
  }
  res.json(latestVehicleState);
});

// GET /api/location/history?limit=50&from=ISO&to=ISO
app.get('/api/location/history', async (req, res) => {
  try {
    const limit = Math.min(parseInt(req.query.limit) || 50, 500);
    const query = {};

    if (req.query.from || req.query.to) {
      query.timestamp = {};
      if (req.query.from) query.timestamp.$gte = new Date(req.query.from);
      if (req.query.to)   query.timestamp.$lte = new Date(req.query.to);
    }

    const records = await Location
      .find(query)
      .sort({ timestamp: -1 })
      .limit(limit)
      .select('latitude longitude speed_kmph direction heading timestamp obstacle_detected')
      .lean();

    res.json({ count: records.length, data: records.reverse() });

  } catch (err) {
    console.error('[API] History query error:', err.message);
    res.status(500).json({ error: 'Database error' });
  }
});

// GET /api/stats
app.get('/api/stats', async (req, res) => {
  try {
    const total  = await Location.countDocuments();
    const agg    = await Location.aggregate([
      {
        $group: {
          _id          : null,
          avgSpeed     : { $avg: '$speed_kmph' },
          maxSpeed     : { $max: '$speed_kmph' },
          totalObstacles: { $sum: { $cond: ['$obstacle_detected', 1, 0] } },
          dirCounts    : { $push: '$direction' }
        }
      }
    ]);

    res.json({
      totalRecords  : total,
      packetsLive   : totalPacketsReceived,
      avgSpeed_kmph : agg[0]?.avgSpeed?.toFixed(2)   || 0,
      maxSpeed_kmph : agg[0]?.maxSpeed?.toFixed(2)   || 0,
      totalObstacleEvents: agg[0]?.totalObstacles || 0
    });

  } catch (err) {
    console.error('[API] Stats error:', err.message);
    res.status(500).json({ error: 'Database error' });
  }
});

// ─────────────────────────────────────────────
//  Socket.IO
// ─────────────────────────────────────────────
io.on('connection', (socket) => {
  console.log(`[WS] Client connected: ${socket.id}`);

  // Send latest state immediately on connect
  if (latestVehicleState) {
    socket.emit('vehicleUpdate', latestVehicleState);
  }

  socket.on('disconnect', () => {
    console.log(`[WS] Client disconnected: ${socket.id}`);
  });

  // Allow dashboard to request history
  socket.on('requestHistory', async (opts) => {
    const limit = Math.min(opts?.limit || 100, 500);
    const records = await Location.find()
      .sort({ timestamp: -1 })
      .limit(limit)
      .lean();
    socket.emit('historyData', records.reverse());
  });
});

// ─────────────────────────────────────────────
//  Database Connection & Server Start
// ─────────────────────────────────────────────
mongoose.connect(MONGO_URI)
  .then(() => {
    console.log('[DB] MongoDB connected:', MONGO_URI);
    server.listen(PORT, () => {
      console.log(`\n🚗 Vehicle Tracking Server running on http://localhost:${PORT}`);
      console.log(`📡 Socket.IO live updates enabled`);
      console.log(`📍 POST data to: http://localhost:${PORT}/api/location\n`);
    });
  })
  .catch(err => {
    console.error('[DB] MongoDB connection failed:', err.message);
    process.exit(1);
  });

// Graceful shutdown
process.on('SIGTERM', () => {
  console.log('[SERVER] Shutting down gracefully...');
  server.close(() => mongoose.connection.close());
});

module.exports = { app, server };