public class MotionRow
{
    // raw values
    public long device_timestamp { get; set; }
    public double ax { get; set; }
    public double ay { get; set; }
    public double az { get; set; }
    public double gx { get; set; }
    public double gy { get; set; }
    public double gz { get; set; }

    // derived
    public double roll { get; set; }
    public double pitch { get; set; }
    public double yawRate { get; set; }
    public double heave { get; set; }
}