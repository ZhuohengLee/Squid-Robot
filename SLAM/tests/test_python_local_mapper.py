import csv
import tempfile
import unittest
from pathlib import Path

from SLAM.python_local_mapper import (
    MapperConfig,
    OccupancyGridMap,
    Pose2D,
    SensorFrame,
    build_map_from_frames,
    load_sensor_frames,
)


class PythonLocalMapperTests(unittest.TestCase):
    def test_synthetic_corridor_fixture_builds_non_empty_map(self) -> None:
        fixture_path = (
            Path(__file__).resolve().parent / "fixtures" / "synthetic_corridor_forward.csv"
        )

        frames = load_sensor_frames(fixture_path)
        result = build_map_from_frames(frames, MapperConfig())

        self.assertEqual(result.frames_processed, 22)
        self.assertGreater(result.final_pose.x_cm, 20.0)
        self.assertGreater(result.known_cells, 100)
        self.assertGreater(result.occupied_cells, 5)

    def test_load_sensor_frames_supports_current_esp32_sensor_csv(self) -> None:
        tests_dir = Path(__file__).resolve().parent
        with tempfile.NamedTemporaryFile(
            mode="w",
            newline="",
            encoding="utf-8",
            suffix=".csv",
            dir=tests_dir,
            delete=False,
        ) as handle:
            csv_path = Path(handle.name)
            writer = csv.DictWriter(
                handle,
                fieldnames=[
                    "millis",
                    "depth_cm",
                    "vz_cms",
                    "az_cms2",
                    "us_front_cm",
                    "us_left_cm",
                    "us_right_cm",
                    "motion",
                    "batt_v",
                ],
            )
            writer.writeheader()
            writer.writerow(
                {
                    "millis": 0,
                    "depth_cm": 10.0,
                    "vz_cms": 0.0,
                    "az_cms2": 0.0,
                    "us_front_cm": 100.0,
                    "us_left_cm": 60.0,
                    "us_right_cm": 62.0,
                    "motion": "forward",
                    "batt_v": 11.9,
                }
            )
            writer.writerow(
                {
                    "millis": 100,
                    "depth_cm": 10.2,
                    "vz_cms": 0.0,
                    "az_cms2": 0.0,
                    "us_front_cm": 95.0,
                    "us_left_cm": 59.0,
                    "us_right_cm": 63.0,
                    "motion": "forward",
                    "batt_v": 11.8,
                }
            )

        try:
            frames = load_sensor_frames(csv_path)
        finally:
            csv_path.unlink(missing_ok=True)

        self.assertEqual(len(frames), 2)
        self.assertEqual(frames[0].timestamp_ms, 0)
        self.assertEqual(frames[1].dt_s, 0.1)
        self.assertEqual(frames[0].motion_mode, "forward")
        self.assertEqual(frames[0].front_cm, 100.0)
        self.assertIsNone(frames[0].gyro_z_deg_s)

    def test_load_sensor_frames_reads_optional_imu_columns(self) -> None:
        tests_dir = Path(__file__).resolve().parent
        with tempfile.NamedTemporaryFile(
            mode="w",
            newline="",
            encoding="utf-8",
            suffix=".csv",
            dir=tests_dir,
            delete=False,
        ) as handle:
            csv_path = Path(handle.name)
            writer = csv.DictWriter(
                handle,
                fieldnames=[
                    "timestamp_ms",
                    "us_front_cm",
                    "us_left_cm",
                    "us_right_cm",
                    "motion_mode",
                    "roll_deg",
                    "pitch_deg",
                    "gyro_z_deg_s",
                ],
            )
            writer.writeheader()
            writer.writerow(
                {
                    "timestamp_ms": 50,
                    "us_front_cm": 90.0,
                    "us_left_cm": 50.0,
                    "us_right_cm": 48.0,
                    "motion_mode": "turn_left",
                    "roll_deg": 1.2,
                    "pitch_deg": -0.3,
                    "gyro_z_deg_s": 16.0,
                }
            )

        try:
            frames = load_sensor_frames(csv_path)
        finally:
            csv_path.unlink(missing_ok=True)

        self.assertEqual(len(frames), 1)
        self.assertEqual(frames[0].motion_mode, "turn_left")
        self.assertAlmostEqual(frames[0].roll_deg or 0.0, 1.2)
        self.assertAlmostEqual(frames[0].pitch_deg or 0.0, -0.3)
        self.assertAlmostEqual(frames[0].gyro_z_deg_s or 0.0, 16.0)

    def test_build_map_from_frames_produces_motion_and_known_cells(self) -> None:
        frames = [
            SensorFrame(
                timestamp_ms=0,
                dt_s=0.1,
                motion_mode="forward",
                front_cm=100.0,
                left_cm=60.0,
                right_cm=60.0,
                roll_deg=0.0,
                pitch_deg=0.0,
                gyro_z_deg_s=0.0,
            ),
            SensorFrame(
                timestamp_ms=100,
                dt_s=0.1,
                motion_mode="forward",
                front_cm=95.0,
                left_cm=60.0,
                right_cm=60.0,
                roll_deg=0.0,
                pitch_deg=0.0,
                gyro_z_deg_s=0.0,
            ),
            SensorFrame(
                timestamp_ms=200,
                dt_s=0.1,
                motion_mode="forward",
                front_cm=90.0,
                left_cm=58.0,
                right_cm=62.0,
                roll_deg=0.0,
                pitch_deg=0.0,
                gyro_z_deg_s=0.0,
            ),
        ]
        config = MapperConfig()

        result = build_map_from_frames(frames, config)

        self.assertGreater(result.final_pose.x_cm, 0.0)
        self.assertGreater(result.known_cells, 0)
        self.assertGreater(result.occupied_cells, 0)

    def test_occupancy_grid_marks_expected_hit_cell(self) -> None:
        grid = OccupancyGridMap(
            width_cells=80,
            height_cells=80,
            resolution_cm=5.0,
            occupied_threshold=3,
        )

        grid.insert_ray(
            origin=Pose2D(0.0, 0.0, 0.0),
            beam_yaw_deg=0.0,
            range_cm=50.0,
            max_range_cm=200.0,
            hit=True,
        )

        hit_value = grid.get_world_value(50.0, 0.0)
        self.assertGreaterEqual(hit_value, grid.occupied_threshold)


if __name__ == "__main__":
    unittest.main()
