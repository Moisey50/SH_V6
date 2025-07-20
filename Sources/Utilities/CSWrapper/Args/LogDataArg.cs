using System;
using System.Collections.Generic;
using System.Linq;

namespace CSWrapper
{
  public class LogDataArg
    : IDataArg<LogDataArg>
    , IData
  {
    public enum LogVerbosity
    {
      Verbose,
      Info,
      Debug,
      Critical,
    }

    #region | Fields |
    private bool _isDisposedValue = false; // To detect redundant calls
    #endregion | Fields |

    #region | Properties |
    public LogDataArg Data => this;

    public IList<Delegate> Delegates { get; }

    public LogVerbosity Verbosity { get; }

    public string SourceName { get; }

    public string Message { get; }

    public DateTime ReceivedAt { get; }
    #endregion | Properties |

    #region | Constractors |
    /// <summary>
    /// 
    /// </summary>
    /// <param name="verbosity"></param>
    /// <param name="srcName"></param>
    /// <param name="msg"></param>
    /// <param name="delegates"></param>
    public LogDataArg(LogVerbosity verbosity, string srcName, string msg, DateTime? receivedAt=null, IList<Delegate> delegates = null)
    {
      ReceivedAt = receivedAt ?? DateTime.Now;
      Verbosity = verbosity;
      SourceName = srcName;
      Message = msg;

      Delegates = delegates;
    }
    #endregion | Constractors |

    #region | Methods |   

    protected virtual void Dispose(bool disposing)
    {
      if (!_isDisposedValue)
      {
        if (disposing)
        {
          // TODO: dispose managed state (managed objects).
        }

        // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
        // TODO: set large fields to null.

        _isDisposedValue = true;
      }
    }
        // TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
    // ~LogData() {
    //   // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
    //   Dispose(false);
    // }

    // This code added to correctly implement the disposable pattern.
    public void Dispose()
    {
      // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
      Dispose(true);
      // TODO: uncomment the following line if the finalize is overridden above.
      // GC.SuppressFinalize(this);
    }

    public override string ToString()
    {
      return $"{ReceivedAt}#{SourceName}#{Message}";
    }

    [Obsolete("The GetData() has been deprecated.  Use Data property instead.", false)]
    public object GetData()
    {
      return Data;
    }
    [Obsolete("The CallList() has been deprecated.  Use Delegates property instead.", false)]
    public List<Delegate> CallList()
    {
      return Delegates?.ToList();
    }
    #endregion | Methods |
  }
}