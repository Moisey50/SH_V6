using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;


using shbaseCLI;
using MfcCoupler;

namespace FLWPlayerSharp
{
    public partial class FLWDialog : Form
    {
        //Looks that it not in use Yuri S (20170910) -- private MfcDllCoupler m_MfcDllCoupler = null;
        private GraphBuilder _builder;
        private const string _graphAsScript = "FLWCapture FLWCaptureGadget1(FileName=E:\\06.30.11 17-19-24.flw;) VideoRender VideoRenderGadget1(Name=Float window 1;Monochrome=false;Scale=-1;) Connect(FLWCaptureGadget1>>0,VideoRenderGadget1<<0) BEGIN_VIEW_SECTION MOVE(gadget=FLWCaptureGadget1;x=98;y=135;) MOVE(gadget=VideoRenderGadget1;x=503;y=191;) END_VIEW_SECTION";
        
        public FLWDialog()
        {
            InitializeComponent();
        }

        private void FLWDialog_Load(object sender, EventArgs e)
        {
            //Looks that it not in use Yuri S (20170910) -- m_MfcDllCoupler = MfcDllCoupler.CreateInstance(new WndManager(this));
            _builder = new GraphBuilder();

            //There are 2 options to load the graph as follow:
            //Option #1 - build graph from the script (e.g: _graphAsScript)
            //MsgLevels result = _builder.Load(null, _graphAsScript);

            //Option #2 - build graph from the '.tvg' file (e.g: @".\Resources\Demo.tvg")
            MsgLevels result = _builder.Load(@".\Resources\Demo.tvg", null);
            if (result >= MsgLevels.MSG_WARNING_LEVEL)
            {
                MessageBox.Show("Can't load graph", "GraphBuilder Error");
            }

            IntPtr disp=displayBox.Handle;
            if (!_builder.ConnectRendererAndMonitor("VideoRenderGadget1", disp, "Float window 1"))
            {
                MessageBox.Show("Can't connect render and monitor", "GraphBuilder Error");
            }
        }

        private void FLWDialog_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (_builder != null)
            {
                _builder.Release();
                _builder.Dispose();
            }
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            OpenFileDialog o = new OpenFileDialog();
            o.Filter = "Stream Handler Packet Files(*.flw)|*.flw|AllFiles|*.*";
            if (o.ShowDialog() == DialogResult.OK)
            {
                if (_builder!=null)
                {
                    _builder.GadgetScanProperties("FLWCaptureGadget1", "FileName=" + o.FileName + ";");
                }
                else
                {
                    MessageBox.Show("Graph builder was not properly initialized", "GraphBuilder Error");
                }
            }
        }

        private void buttonSetup_Click(object sender, EventArgs e)
        {
            if (_builder != null)
                _builder.RunSetupDialog();
        }

        private void buttonRun_Click(object sender, EventArgs e)
        {
            if (_builder != null)
                _builder.Start();
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            if (_builder != null)
                _builder.Stop();
        }


        internal class WndManager : IWndInspector
        {
            Form m_MainWnd;
            public WndManager(Form form)
            {
                m_MainWnd = form;
            }
            #region IWndManager Members
            public IntPtr GetMainWnd()
            {
                System.Diagnostics.Debug.Assert(m_MainWnd.IsHandleCreated);
                return m_MainWnd.Handle;
            }
            #endregion
        }

    }
}
