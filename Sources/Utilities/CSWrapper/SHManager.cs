using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using shbaseCLI;
using System.Windows;
using System.Windows.Interop;
using System.Runtime.InteropServices;
using Microsoft.Win32;
using System.Collections;
using System.Threading;
using System.Collections.Concurrent;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Runtime.Remoting.Messaging;

namespace CSWrapper
{
  [UnmanagedFunctionPointer(CallingConvention.StdCall)]
  delegate void ManagedCB(IntPtr msg, int msgLength, IntPtr pinName, int pinNameLength, uint h);

  [UnmanagedFunctionPointer(CallingConvention.StdCall)]
  delegate void ManagedLogMsgCB(int msgLevel, IntPtr src, int srcLength, int msgId, IntPtr msgText, int msgTextLength, uint h);


  public delegate void GetDataDelegate(IData data);

  class MethodsCollection
  {
    private Dictionary<IntPtr, Delegate> _infos;

    public void Add(IntPtr h, Delegate del)
    {
      if (_infos == null)
        _infos = new Dictionary<IntPtr, Delegate>();

      if (_infos.ContainsKey(h))
        _infos[h] = del;
      else
        _infos.Add(h, del);
    }
    public int Remove(IntPtr h)
    {
      if (_infos.ContainsKey(h))
        _infos.Remove(h);

      return _infos.Count;
    }
    public int Count()
    {
      return _infos.Count;
    }

    public Delegate GetDelegateByHandle(IntPtr h)
    {
      if (_infos.ContainsKey(h))
        return _infos[h];
      else
        return null;
    }

    public List<Delegate> GetDelegatesCollection()
    {
      List<Delegate> list = new List<Delegate>();
      foreach (var i in _infos)
      {
        list.Add(i.Value);
      }
      return list;
    }
  }
  class Pin
  {
    public string Name { get; set; }
    public MethodsCollection Methods { get; private set; }
  }

  public sealed class MFCSHBuilder : IDisposable
  {
    #region | Events |
    public event EventHandler Initialized;
    public event EventHandler Loaded;
    public event EventHandler Started;
    public event EventHandler Stoped;

    public event EventHandler<LogDataArg> LogReceived;
    #endregion | Events |

    #region | Fields |
    private static MFCSHBuilder _instance;
    private static readonly object _lock = new object();
    //private static AutoResetEvent _signal = null;
    private static BackgroundWorker _bw = null;


    private GraphBuilder _builder;

    Dictionary<string, MethodsCollection> _delegatesPerPinCollection;
    private List<GCHandle> _delegateHandles;
    private GetDataDelegate _logMsgDel;
//     //20180310 Yuri - 'CallbackOnCollectedDelegate' error handling
//     // <cref=https://dzimchuk.net/ouch-callbackoncollecteddelegate-was-detected/ />
//     private ManagedLogMsgCB _managedLogMsgCB = null;

    private readonly ConcurrentQueue<IData> _queue = new ConcurrentQueue<IData>();
    #endregion | Fields |

    #region | Properties |
    /// <summary>
    /// Loaded Graph's full path.
    /// </summary>
    public string LoadedGraphFullPath { get; private set; }

    /// <summary>
    /// If the graph at <cref!=LoadedGraphFullPath/> is loaded.
    /// </summary>
    public bool IsGraphLoaded => !string.IsNullOrEmpty(LoadedGraphFullPath) && File.Exists(LoadedGraphFullPath);

    /// <summary>
    /// If the graph at <cref!=LoadedGraphFullPath/> is started and running.
    /// </summary>
    public bool IsRunning { get; private set; }
    #endregion | Properties |

    #region | Constructor |
    /// <summary>
    /// Default constructor.
    /// </summary>
    private MFCSHBuilder()
    {
      LoadedGraphFullPath = null;
      IsRunning = false;
    }
    #endregion | Constructor |

    #region | Methods |
    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    public static MFCSHBuilder Instance()
    {
      if (_instance == null)
      {
        lock (_lock)
        {
          // create the instance only if the instance is null
          if (_instance == null)
          {
            _instance = new MFCSHBuilder();

          }
        }
      }
      // Otherwise return the already existing instance
      return _instance;
    }

    public void Dispose()
    {
      if (_bw != null && _bw.IsBusy)
      {
        _bw.CancelAsync();
        //if (_signal != null)
        //{
        //    _signal.Set();                    
        //    //_signal = null;
        //}
        //_bw = null;
      }


      if (_delegatesPerPinCollection != null)
      {
        //TODO: Check what to do before Clear();
        foreach (var pinDelegates in _delegatesPerPinCollection)
        {
          pinDelegates.Value.GetDelegatesCollection().Clear();
        }
        _delegatesPerPinCollection.Clear();
        _delegatesPerPinCollection = null;
      }

      if (_queue != null)
      {
        IData item = null;

        while (!_queue.IsEmpty)
          if (_queue.TryDequeue(out item))
            item.Dispose();
      }

      if ((_delegateHandles?.Count).GetValueOrDefault() > 0)
      {
        foreach (var gch in _delegateHandles)
        {
          gch.Free();
        }

        _delegateHandles.Clear();
      }
      _delegateHandles = null;

//       //20180310 Yuri - 'CallbackOnCollectedDelegate' error handling
//       // <cref=https://dzimchuk.net/ouch-callbackoncollecteddelegate-was-detected/ />
//       if (_managedLogMsgCB != null)
//         _managedLogMsgCB = null;

      if (_logMsgDel != null)
        _logMsgDel = null;

      if (_builder != null)
      {
        //is unneccessary (already in Release()) - yura20180710 _builder.Stop();
        _builder.Release();
        _builder = null;
      }

    }

    private void InitLocalListener()
    {
      if (/*_signal == null &&*/ _bw == null)
      {
        //_signal = new AutoResetEvent(false);
        _bw = new BackgroundWorker
        {
          WorkerSupportsCancellation = true
        };
        _bw.DoWork += DoListen;
        _bw.RunWorkerAsync();
      }
    }

    private void DoListen(object sender, DoWorkEventArgs e)
    {
      while (!_bw.CancellationPending)
      {
        //_signal.WaitOne();
        if (_queue.Count == 0)
          Thread.Sleep(500);
        else
        {
          IData item = null;
          bool isDequeued = false;
          lock (_lock)
          {
            isDequeued = _queue.TryDequeue(out item);
          }

          if (isDequeued && item != null)
          {
            foreach (var del in item.CallList())
            {
              del.DynamicInvoke(item/*.GetData()*/);
            }
          }
        }
      }

      //_signal.Close();
      //_signal = null;
      _bw = null;
    }

    /// <summary>
    /// Initialize Stream Handler environment.
    /// </summary>
    /// <param name="msgLogDel"></param>
    public void Init(GetDataDelegate msgLogDel = null)
    {
      if (_delegateHandles == null)
        _delegateHandles = new List<GCHandle>();

      if (_delegatesPerPinCollection == null)
        _delegatesPerPinCollection = new Dictionary<string, MethodsCollection>();

      if (_builder == null)
      {
        //         if (msgLogDel == null)
        //           _builder = new GraphBuilder();
        //         else
        //         {
                  _logMsgDel = msgLogDel ?? LocalLogsMsgsHandler;
        //
        //         //20180310 Yuri - 'CallbackOnCollectedDelegate' error handling
        //         // <cref=https://dzimchuk.net/ouch-callbackoncollecteddelegate-was-detected/ />
        //         ManagedLogMsgCB managedLogMsgCB = LogMsgCallBack;
        

        IntPtr stringStdCallDelegatePointer = GetNativePointerToCallback(LogMsgCallBack);
        if (stringStdCallDelegatePointer != IntPtr.Zero && (_delegateHandles?.Count).GetValueOrDefault() > 0)
          InitLocalListener();

        _builder = new GraphBuilder(stringStdCallDelegatePointer);

        //         }
      }

      if (IsBuilderExist())
        Initialized?.Invoke(this, EventArgs.Empty);
    }

    private void LocalLogsMsgsHandler(IData logData)
    {
      OnLogMessageReceived(logData);
    }

    private void OnLogMessageReceived(IData logData)
    {
      if (logData is LogDataArg log)
        LogReceived?.Invoke(this, new LogDataArg(log.Data.Verbosity, log.SourceName, log.Message, log.ReceivedAt));
    }

    [Obsolete("The 'CreateInstance(...)' function has been deprecated. Use the 'Init(...)' function instead.", false)]
    public void CreateInstance(Window wnd, /*GetStringDelegate*/GetDataDelegate msgLogDel = null)
    {
      //20180310 Yuri - unnecessary
      //if (_MfcDllCoupler == null)
      //    _MfcDllCoupler = MfcDllCoupler.CreateInstance(new WndManager(wnd));

      if (_delegateHandles == null)
        _delegateHandles = new List<GCHandle>();

      if (_delegatesPerPinCollection == null)
        _delegatesPerPinCollection = new Dictionary<string, MethodsCollection>();

      if (_builder == null)
      {
        if (msgLogDel != null)
        {
          _logMsgDel = msgLogDel;

          //           //20180310 Yuri - 'CallbackOnCollectedDelegate' error handling
          //           // <cref=https://dzimchuk.net/ouch-callbackoncollecteddelegate-was-detected/ />
          //           _managedLogMsgCB = LogMsgCallBack;
          // 
          //           //20180310 Yuri - 'CallbackOnCollectedDelegate' error handling
          //           // <cref=https://dzimchuk.net/ouch-callbackoncollecteddelegate-was-detected/ />
          //           ManagedLogMsgCB logMsgCallDelegate = new ManagedLogMsgCB(_managedLogMsgCB);
          //           IntPtr stringStdCallDelegatePointer = Marshal.GetFunctionPointerForDelegate(logMsgCallDelegate);
          //           GCHandle gchCallbackDelegate = GCHandle.Alloc(logMsgCallDelegate);
          //           GC.Collect();
          IntPtr stringStdCallDelegatePointer = GetNativePointerToCallback(LogMsgCallBack);
          _builder = new GraphBuilder(stringStdCallDelegatePointer);
//           _delegateHandles.Add(gchCallbackDelegate);
        }
        else
          _builder = new GraphBuilder();
      }
    }

    /// <summary>
    /// Loads graph from the <cref!=filename/> location into the <cref!=Stream Handler/>.
    /// </summary>
    /// <param name="filename"></param>
    /// <returns></returns>
    public bool LoadGraph(string filename)
    {
      bool res = _builder?.Load(filename) > MsgLevels.MSG_WARNING_LEVEL;
      if (!res)
      {
        LoadedGraphFullPath = filename;
        Loaded?.Invoke(this, EventArgs.Empty);
      }

      return res;
    }
    public bool IsBuilderExist()
    {
      return _builder != null;
    }
    public void StartGraph()
    {
      if (_builder != null && !IsRunning)
      {
        _builder.Start();
        IsRunning = true;
        Started?.Invoke(this, EventArgs.Empty);
      }
    }
    public void StopGraph()
    {
      if (_builder != null)
      {
        _builder.Stop();
        IsRunning = false;
        Stoped?.Invoke(this, EventArgs.Empty);
      }
    }
    public bool OpenGraph(string uid, string text)
    {
      OpenFileDialog o = new OpenFileDialog
      {
        Filter = "Stream Handler Packet Files(*.flw)|*.flw|AllFiles|*.*"
      };

      if (o.ShowDialog() == true)
      {
        if (_builder != null)
        {
          _builder.GadgetScanProperties(uid, text + o.FileName + ";");
          return false;
        }
        else
        {
          return true;
        }
      }
      return true;
    }


    [Obsolete("The RunSetupDialog() function has been deprecated. Use the ShowGraphSetup() function instead.", false)]
    public bool RunSetupDialog()
    {
      return !ShowGraphSetup();
    }

    public bool ShowGraphSetup()
    {
      bool res = false;
      try
      {
        if (IsBuilderExist())
        {
          _builder.RunSetupDialog();
        }
      }
      catch (Exception ex)
      {
        throw new Exception("Show the '{0}' graph setup dialog window has been terminated. See inner exception.", ex);
      }
      return res;
    }

    public bool ShowGadgetSetup(string gadgetName, Point pt)
    {
      bool res = false;
      try
      {
        if (IsBuilderExist())
        {
          _builder.ShowGadgetSetupDlg(gadgetName, (int)pt.X, (int)pt.Y);
          res = true;
        }
      }
      catch (Exception ex)
      {
        throw new Exception("Show the '{0}' gadget setup dialog window has been terminated. See inner exception.", ex);
      }
      return res;
    }

    public bool ConnectRendererAndMonitor(string uid, IntPtr pParentWnd, string Monitor)
    {
      if (!_builder.ConnectRendererAndMonitor(uid, pParentWnd, Monitor))
      {
        return true;
      }
      else
        return false;
    }

    //public static bool ResizeRendererContent(IntPtr pContentView, System.Drawing.Size newSize)
    //{
    //    return GraphBuilder.ResizeRendererContent(pContentView, newSize.Width, newSize.Height);
    //}

    public bool SendBoolean(string pinName, ValueType value, string label = null)
    {
      if (_builder != null)
        return _builder.SendBoolean(pinName, value, label);
      else
        return false;
    }
    public bool SendText(string pinName, string data, string label = null)
    {
      if (_builder != null)
        return _builder.SendText(pinName, data, label);
      else
        return false;
    }
    public bool SendQuantity(string pinName, double data, string Label = null)
    {
      if (_builder != null)
        return _builder.SendQuantity(pinName, data, Label);
      else
        return false;
    }

    public bool SetWorkingMode(string gadgetName, int workingMode)
    {
      if (_builder != null)
        return _builder.SetWorkingMode(gadgetName, workingMode);
      else
        return false;
    }

    /// <summary>
    /// Uploads batch of the <cref!=gadgetName/> gadget's properties with values;
    /// </summary>
    /// <param name="gadgetName">Gadget name</param>
    /// <param name="propertiesWithValues">Batch of properties with values</param>
    /// <returns>Returns true when it successful, otherwise false.</returns>
    public bool SetProperties(string gadgetName, string propertiesWithValues)
    {
      if (_builder != null)
        return _builder.SetProperties(gadgetName, propertiesWithValues);
      return false;
    }

    public bool SetProperty(string gadgetName, string propertyName, string propertyValue)
    {
      if (_builder != null)
        return _builder.SetProperty(gadgetName, propertyName, propertyValue);
      else
        return false;
    }

    public bool SetProperty(string gadgetName, string propertyName, double propertyValue)
    {
      bool? res = false;
      if (_builder != null)
        res = _builder?.SetProperty(gadgetName, propertyName, propertyValue);

      return res.HasValue && res.Value;
    }

    public bool SetProperty(string gadgetName, string propertyName, int propertyValue)
    {
      bool? res = false;
      if (_builder != null)
        res = _builder?.SetProperty(gadgetName, propertyName, propertyValue);

      return res.HasValue && res.Value;
    }

    public bool SetProperty(string gadgetName, string propertyName, bool propertyValue)
    {
      bool? res = false;
      if (_builder != null)
        res = _builder?.SetProperty(gadgetName, propertyName, propertyValue);

      return res.HasValue && res.Value;
    }

    /// <summary>
    /// Extracts from the gadget <cref!=gadgetName/> ALL properties with values as single text;
    /// </summary>
    /// <param name="gadgetName">Gadget name</param>
    /// <param name="allPropWithVals">Container for ALL properties with values</param>
    /// <returns>Returns true when it successful, otherwise false.</returns>
    public bool GetProperties(string gadgetName, ref string allPropWithVals)
    {
      string res = null;
      if (_builder != null)
        res = _builder.GetProperties(gadgetName);
      allPropWithVals = res;

      return !string.IsNullOrEmpty(res);
    }

    public bool GetIntProperty(string GadgetName, string PropertyName, ref int retVal)
    {
      if (_builder != null)
      {
        retVal = _builder.GetIntProperty(GadgetName, PropertyName);
        return true;
      }
      else
        return false; ;
    }

    public bool GetDoubleProperty(string GadgetName, string PropertyName, ref double retVal)
    {
      if (_builder != null)
      {
        retVal = _builder.GetDoubleProperty(GadgetName, PropertyName);
        return true;
      }
      else
        return false; ;
    }

    public bool GetStrtingProperty(string GadgetName, string PropertyName, ref string retVal)
    {
      if (_builder != null)
      {
        var res = _builder.GetStringProperty(GadgetName, PropertyName);
        retVal = string.Copy(res);
        GC.SuppressFinalize(res);
        return true;
      }
      else
        return false; ;
    }

    public bool GetBoolProperty(string GadgetName, string PropertyName, ref bool retVal)
    {
      if (_builder != null)
      {
        int val = _builder.GetBooleanProperty(GadgetName, PropertyName);
        retVal = val == 1 ? true : false;
        return true;
      }
      else
        return false; ;
    }

    List<Delegate> GetDelegates(string pinName)
    {
      if (_delegatesPerPinCollection == null) return null;
      if (_delegatesPerPinCollection.ContainsKey(pinName))
      {
        MethodsCollection c = _delegatesPerPinCollection[pinName];
        return c?.GetDelegatesCollection();
      }
      return null;
    }
    #endregion | Methods |

    #region |SetCallback|
    public bool SetStringCallback(string PinName, GetDataDelegate del, bool bMulti = false)
    {
      bool res = false;

      if (AddListener(PinName, del, bMulti))
      {
//         ManagedCB stringStdCallDelegate = /*new ManagedCB(*/TextDataCallback/*)*/;
//         IntPtr stringStdCallDelegatePointer = Marshal.GetFunctionPointerForDelegate(stringStdCallDelegate);
//         GCHandle gchCallbackDelegate = GCHandle.Alloc(stringStdCallDelegate);
//         GC.Collect(); // create max space for unmanaged allocations

        IntPtr stringStdCallDelegatePointer = GetNativePointerToCallback(TextDataCallback);
        bool? bret = _builder?.SetCharIntPtrCallback(PinName, stringStdCallDelegatePointer, (uint)del.Method.MethodHandle.Value);
//         _delegateHandles.Add(gchCallbackDelegate);
        res = bret.HasValue && bret.Value;

      }

      return res;
    }
    public bool SetFigureCallback(string PinName, GetDataDelegate del, bool bMulti = false)
    {
      bool res = false;

      if (AddListener(PinName, del, bMulti))
      {
//         ManagedCB figureStdCallDelegate = new ManagedCB(DoubleDataCallback);
//         IntPtr figureStdCallDelegatePointer = Marshal.GetFunctionPointerForDelegate(figureStdCallDelegate);
//         GCHandle gchCallbackDelegate = GCHandle.Alloc(figureStdCallDelegate);
//         GC.Collect(); // create max space for unmanaged allocations

        IntPtr figureStdCallDelegatePointer = GetNativePointerToCallback(DoubleDataCallback);
        bool? bret = _builder?.SetFigureIntPtrCallback(PinName, figureStdCallDelegatePointer, (uint)del.Method.MethodHandle.Value);
//         _delegateHandles.Add(gchCallbackDelegate);
        res = bret.HasValue && bret.Value;

      }

      return res;
    }
    #endregion |SetCallback|


    private bool AddListener(string PinName, Delegate del, bool bMulti)
    {
      InitLocalListener();
      bool needRegistration = true;
      MethodsCollection tmp;
      if (_delegatesPerPinCollection.ContainsKey(PinName))
      {
        if (bMulti)
        {
          tmp = _delegatesPerPinCollection[PinName];
          tmp.Add(del.Method.MethodHandle.Value, del);
          needRegistration = false;
        }
        else
        {
          tmp = new MethodsCollection();
        }
        tmp.Add(del.Method.MethodHandle.Value, del);
        _delegatesPerPinCollection[PinName] = tmp;
      }
      else
      {
        tmp = new MethodsCollection();
        tmp.Add(del.Method.MethodHandle.Value, del);
        _delegatesPerPinCollection.Add(PinName, tmp);
      }
      return needRegistration;
    }


    #region |UnSetCallback|
    /// <summary>
    /// 
    /// </summary>
    /// <param name="pinName"></param>
    /// <param name="d"></param>
    [Obsolete("The 'UnSetCharPtrCallback(...)' function has been deprecated. Use the 'RemoveCallback(...)' function instead.", false)]
    public void UnSetCharPtrCallback(string pinName, Delegate d)
    {
      if (_delegatesPerPinCollection.ContainsKey(pinName))
      {
        var v = _delegatesPerPinCollection[pinName];
        if (v.Remove(d.Method.MethodHandle.Value) == 0)
          _builder.UnSetCharPtrCallback(pinName);
      }
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="pinName"></param>
    /// <param name="d"></param>
    [Obsolete("The 'UnSetDoublePtrCallback(...)' function has been deprecated. Use the 'RemoveCallback(...)' function instead.", false)]
    public void UnSetDoublePtrCallback(string pinName, Delegate d)
    {
      if (_delegatesPerPinCollection.ContainsKey(pinName))
      {
        var v = _delegatesPerPinCollection[pinName];
        if (v.Remove(d.Method.MethodHandle.Value) == 0)
          _builder.UnSetCharPtrCallback(pinName);
      }
    }

    /// <summary>
    /// Removes the callbak listener on the pin specified as <cref!=pinName/>.
    /// </summary>
    /// <param name="pinName"></param>
    /// <param name="del"></param>
    public void RemoveCallback(string pinName, GetDataDelegate del)
    {
      if (_delegatesPerPinCollection != null && _delegatesPerPinCollection.ContainsKey(pinName))
      {
        var v = _delegatesPerPinCollection[pinName];
        if (v.Remove(del.Method.MethodHandle.Value) == 0)
          _builder.UnSetCharPtrCallback(pinName);
      }
    }
    #endregion |UnSetCallback|

    #region | Callbacks |

    void LogMsgCallBack(int msgLevel, IntPtr src, int srcLength, int msgId, IntPtr msgText, int msgTextLength, uint h)
    {
      string srcName = GetString(src, srcLength);
      string logMsg = GetString(msgText, msgTextLength);

      Trace.WriteLine($"SHManager >< {srcName}-{logMsg}");
      if (_logMsgDel != null)
      {
        var logData = new LogDataArg((LogDataArg.LogVerbosity)msgLevel, srcName, logMsg, null, (new Delegate[] { _logMsgDel }).ToList());

        //       LogReceived?.BeginInvoke(this, logData, new AsyncCallback(LogReceivedCallback), null);
        //       _logMsgDel?.Invoke(logData);

        lock (_lock)
        {
          _queue.Enqueue(logData);
        }
      }
    }

    void TextDataCallback(IntPtr msg, int msgLength, IntPtr pinName, int pinNameLength, uint h)
    {
      var data = GetString(msg, msgLength);
      var pName = GetString(pinName, pinNameLength);

      var list = GetDelegates(pName);
      StringDataArg item = new StringDataArg(data, list);
      lock (_lock)
      {
        _queue.Enqueue(item);
      }
    }

    void DoubleDataCallback(IntPtr pData, int length, IntPtr pinName, int pinNameLength, uint h)
    {
      var data = GetNumberArrayByPtr(pData, length);
      var pName = GetString(pinName, pinNameLength);

      var localdata = new double[length];
      Array.Copy(data, localdata, length);

      Point[] res = GetPointArrFromText(data);
      var list = GetDelegates(pName);

      FiguresArrayDataArg item = new FiguresArrayDataArg(res, list);
      lock (_lock)
      {
        _queue.Enqueue(item);
      }
    }


    // The callback method must have the same signature as the
    // AsyncCallback delegate.
    private void LogReceivedCallback(IAsyncResult ar)
    {
      // Retrieve the delegate.
      AsyncResult result = (AsyncResult)ar;
      EventHandler<LogDataArg> caller = result.AsyncDelegate as EventHandler<LogDataArg>;

      // Retrieve the format string that was passed as state 
      // information.
      string formatString = (string)ar.AsyncState;

      // Call EndInvoke to retrieve the results.
      caller.EndInvoke(ar);
    }

    private Point[] GetPointArrFromText(double[] localData)
    {
      List<Point> ptList = new List<Point>();
      for (int i = 0; i < localData.Length - 1;)
      {
        ptList.Add(new Point(localData[i], localData[i + 1]));
        i = i + 2;
      }
      return ptList.ToArray();
    }


    string GetString(IntPtr ptrOnStr, int length)
    {
      string res = null;
      byte[] managedBytedArray = new byte[length];

      Marshal.Copy(ptrOnStr, managedBytedArray, 0, length);
      Marshal.FreeHGlobal(ptrOnStr);

      res = Encoding.ASCII.GetString(managedBytedArray);

      return res;
    }

    double[] GetNumberArrayByPtr(IntPtr d, int length)
    {
      double[] managedArray = new double[length];
      Marshal.Copy(d, managedArray, 0, length);
      return managedArray;
    }


    private IntPtr GetNativePointerToCallback(ManagedLogMsgCB managedCallback)
    {
      IntPtr res = IntPtr.Zero;
      ManagedLogMsgCB callbackManagedCopy = null;

      //20180310 Yuri - 'CallbackOnCollectedDelegate' error handling
      // <cref=https://dzimchuk.net/ouch-callbackoncollecteddelegate-was-detected/ />
      callbackManagedCopy = new ManagedLogMsgCB(managedCallback);

      if (callbackManagedCopy != null)
      {
        res = Marshal.GetFunctionPointerForDelegate(callbackManagedCopy);
        GCHandle gchCallbackDelegate = GCHandle.Alloc(callbackManagedCopy);
        GC.Collect();  // create max space for unmanaged allocations
        _delegateHandles.Add(gchCallbackDelegate);
      }
      return res;
    }

    private IntPtr GetNativePointerToCallback(ManagedCB managedCallback)
    {
      IntPtr res = IntPtr.Zero;
      ManagedCB callbackManagedCopy = null;

      //20180310 Yuri - 'CallbackOnCollectedDelegate' error handling
      // <cref=https://dzimchuk.net/ouch-callbackoncollecteddelegate-was-detected/ />
      callbackManagedCopy = new ManagedCB(managedCallback);

      if (callbackManagedCopy != null)
      {
        res = Marshal.GetFunctionPointerForDelegate(callbackManagedCopy);
        GCHandle gchCallbackDelegate = GCHandle.Alloc(callbackManagedCopy);
        GC.Collect();  // create max space for unmanaged allocations
        _delegateHandles.Add(gchCallbackDelegate);
      }
      return res;
    }
    #endregion | Callbacks |
  }
}