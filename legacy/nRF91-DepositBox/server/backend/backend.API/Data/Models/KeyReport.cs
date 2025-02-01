using System;
using Microsoft.EntityFrameworkCore;

public enum KeyReportStatus
{
    Gone,
    Present,
}

namespace backend.API.Data.Models
{
    [Index(nameof(KeyReportID), IsUnique = true)]
    public class KeyReport
    {
        public long KeyReportID { get; set; }
        public LockBox LockBox { get; set; }
        public DateTime TimeStamp { get; set; }
        public KeyReportStatus Status { get; set; }
    }
}