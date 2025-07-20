using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace CSWrapper
{
  /// <summary>
  /// 
  /// </summary>
  public class FiguresArrayDataArg
    : IDataArg<Point[]>
    , IData
  {
    //List<Delegate> _callList;
    //public Point[] _data;
    #region | Properties |
    /// <summary>
    /// 
    /// </summary>
    public Point[] Data { get; }

    /// <summary>
    /// 
    /// </summary>
    public IList<Delegate> Delegates { get; }
    #endregion | Properties |

    #region | Constructors |
    /// <summary>
    /// 
    /// </summary>
    /// <param name="arr"></param>
    /// <param name="list"></param>
    public FiguresArrayDataArg(Point[] arr, IList<Delegate> list)
    {
      Delegates = list;

      var data = new Point[arr.Length];
      Array.Copy(arr, data, arr.Length);
      Data = data;
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
      return $"{Data?.Length}";
    }
    #endregion | Methods |
  }
}

