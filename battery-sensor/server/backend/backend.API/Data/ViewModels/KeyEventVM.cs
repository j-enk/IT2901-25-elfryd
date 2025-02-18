using System;

public class KeyEventVM
{
    public long LockBoxID { get; set; }
    public DateTime TimeStamp { get; set; }
    public KeyEventActions Action { get; set; }
    public string UserName { get; set; }
}