using System;
using System.Collections.Generic;

namespace CSWrapper
{
  /// <summary>
  /// 
  /// </summary>
  [Obsolete("The IData interface has been deprecated.  Use IDataArg<TData> instead.", false)]
  public interface IData : IDisposable
  {
    object GetData();
    List<Delegate> CallList();
  }

}