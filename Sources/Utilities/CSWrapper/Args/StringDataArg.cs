using System;
using System.Collections.Generic;
using System.Linq;

namespace CSWrapper
{
  public class StringDataArg
    : IDataArg<string>
    , IData
  {
    //         List<Delegate> _callList;
    //         public string _data;

    #region | Properties |
    public string Data { get; }

    public IList<Delegate> Delegates { get; }
    #endregion | Properties |

    #region | Constructors |
    public StringDataArg(string str, IList<Delegate> list = null)
    {
      Delegates = list;
      Data = String.Copy(str);
    }
    #endregion | Constructors |

    #region | Methods |
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

    public void Dispose()
    {

    }

    public override string ToString()
    {
      return $"{Data}";
    }
    #endregion | Methods |
  }
}
