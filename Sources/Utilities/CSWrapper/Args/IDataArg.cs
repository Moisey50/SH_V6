using System;
using System.Collections.Generic;

namespace CSWrapper
{
  /// <summary>
  /// 
  /// </summary>
  /// <typeparam name="TData"></typeparam>
  public interface IDataArg<TData> : IDisposable
  {
    TData Data { get; }
    IList<Delegate> Delegates { get; }
  }

}