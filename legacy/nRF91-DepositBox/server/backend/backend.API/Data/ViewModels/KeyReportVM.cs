
using System;

namespace backend.API.Data.ViewModels
{
    public class KeyReportVM
    {
        public long KeyReportID { get; set; }
        public long LockBoxID { get; set; }
        public DateTime TimeStamp { get; set; }
        public KeyReportStatus Status { get; set; }
    }
}