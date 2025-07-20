#ifndef VIDEO_FILTER_TEMPLATE_INCLUDED
#define VIDEO_FILTER_TEMPLATE_INCLUDED

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
	if (!(vfr) || ((vfr)->IsNullFrame()))	    \
    {	                                        \
		return NULL;			                \
    }                                           \
}

#define DECLARE_VIDEO_FILTER(class_name, declare_params)			\
	class class_name : public CFilterGadget							\
	{																\
		declare_params;												\
	public:															\
		virtual void ShutDown();								    \
	private:														\
		class_name();												\
		virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);	\
		DECLARE_RUNTIME_GADGET(class_name);							\
	};																\

#define DECLARE_VIDEO_FILTER_WITH_SETUP(class_name, declare_params, dlg_class_name)			\
	class class_name : public CFilterGadget							\
	{																\
		dlg_class_name* m_pSetupDlg;								\
		declare_params;												\
	public:															\
		virtual void ShutDown();								    \
		virtual void ShowSetupDialog(CPoint& point);	            \
		virtual BOOL IsSetupOn();									\
		virtual bool PrintProperties(FXString& text);				\
		virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);\
        virtual bool ScanSettings(FXString& text)                   \
        {                                                           \
            text="calldialog(true)";                                \
            return true;                                            \
        }                                                           \
	private:														\
		class_name();												\
		virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);	\
		friend class dlg_class_name;								\
		DECLARE_RUNTIME_GADGET(class_name);							\
	};																\

#define DECLARE_VIDEO_FILTER_WITH_SETUP2(class_name, declare_params, dlg_class_name, add_members)			\
	class class_name : public CFilterGadget							\
	{																\
		dlg_class_name* m_pSetupDlg;								\
		declare_params;												\
	public:															\
		virtual void ShutDown();								    \
		virtual void ShowSetupDialog(CPoint& point);	            \
		virtual BOOL IsSetupOn();									\
		virtual bool PrintProperties(FXString& text);				\
		virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);\
        add_members;                                                \
	private:														\
		class_name();												\
		virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);	\
		friend class dlg_class_name;								\
		DECLARE_RUNTIME_GADGET(class_name);							\
	};																\

#define DECLARE_VIDEO_FILTER_WITH_STDSETUP(class_name, declare_params)	\
	class class_name : public CFilterGadget							\
	{																\
		declare_params;												\
	public:															\
		virtual void ShutDown();								    \
		virtual bool PrintProperties(FXString& text);				\
		virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);\
        virtual bool ScanSettings(FXString& text);                   \
	private:														\
		class_name();												\
		virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);	\
		DECLARE_RUNTIME_GADGET(class_name);							\
	};																\

#define DECLARE_VFILTER_WITH_STDSETUP_ANDASYNC(\
  class_name, declare_params)	\
	class class_name : public CFilterGadget							\
	{																\
		declare_params;												\
    CDuplexConnector * m_pControl ; \
	public:															\
		virtual void ShutDown();								    \
		virtual bool PrintProperties(FXString& text);				\
		virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);\
    virtual bool ScanSettings(FXString& text);                   \
    virtual CDuplexConnector* GetDuplexConnector( int iConnNum ) \
      { return iConnNum == 0 ? m_pControl : NULL ; } ; \
    virtual int GetDuplexCount()  { return (m_pControl)?1:0; } ;\
    virtual void AsyncTransaction( \
      CDuplexConnector* pConnector , CDataFrame* pParamFrame ) ;\
\
	private:														\
		class_name();												\
		virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);	\
		DECLARE_RUNTIME_GADGET(class_name);							\
	};																\

	
#define IMPLEMENT_VIDEO_FILTER(class_name, lineage, plugin, init_params, delete_params)	\
	IMPLEMENT_RUNTIME_GADGET_EX(class_name, CFilterGadget, "Video." lineage, plugin);\
	class_name::class_name()										\
	{																\
		init_params;												\
		m_pInput = new CInputConnector(vframe);						\
		m_pOutput = new COutputConnector(vframe);					\
		Resume();			    									\
	}																\
	void class_name::ShutDown()										\
	{																\
		CFilterGadget::ShutDown();									\
		delete m_pInput;											\
		m_pInput = NULL;											\
		delete m_pOutput;											\
		m_pOutput = NULL;											\
		delete_params;												\
	}																\

#define IMPLEMENT_VIDEO_FILTER_WITH_SETUP(class_name, lineage, plugin, init_params, delete_params)	\
	IMPLEMENT_RUNTIME_GADGET_EX(class_name, CFilterGadget, "Video." lineage, plugin);\
	class_name::class_name():										\
	m_pSetupDlg(NULL)												\
	{																\
		init_params;												\
		m_pInput = new CInputConnector(vframe);						\
		m_pOutput = new COutputConnector(vframe);					\
		Resume();													\
	}																\
    void class_name::ShutDown()										\
	{																\
        AFX_MANAGE_STATE(AfxGetStaticModuleState());                \
		if (m_pSetupDlg->GetSafeHwnd( ))                            \
            m_pSetupDlg->DestroyWindow();                           \
        if (m_pSetupDlg)											\
			m_pSetupDlg->Delete();									\
		m_pSetupDlg = NULL;											\
        CFilterGadget::ShutDown();									\
		delete m_pInput;											\
		m_pInput = NULL;											\
		delete m_pOutput;											\
		m_pOutput = NULL;											\
		delete_params;												\
	}																\
	BOOL class_name::IsSetupOn()									\
	{																\
		return ((m_pSetupDlg != NULL) && m_pSetupDlg->IsOn());		\
	}																\

#define IMPLEMENT_VFILTER_WITH_STDSETUP_ANDASYNC(class_name, lineage, plugin, init_params, delete_params)	\
	IMPLEMENT_RUNTIME_GADGET_EX(class_name, CFilterGadget, \
    "Video." lineage, plugin);\
	class_name::class_name()										\
	{																\
		init_params;     \
		m_pControl = new CDuplexConnector( this );	\
		m_pInput = new CInputConnector(vframe);						\
		m_pOutput = new COutputConnector(vframe);					\
		Resume();													\
	}																\
    void class_name::ShutDown()										\
	{																\
		CFilterGadget::ShutDown();									\
    delete m_pControl ; \
    m_pControl = NULL ;\
		delete m_pInput;											\
		m_pInput = NULL;											\
		delete m_pOutput;											\
		m_pOutput = NULL;											\
		delete_params;												\
	}																\

#define IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP(class_name, lineage, plugin, init_params, delete_params)	\
	IMPLEMENT_RUNTIME_GADGET_EX(class_name, CFilterGadget, "Video." lineage, plugin);\
	class_name::class_name()										\
	{																\
		init_params;												\
		m_pInput = new CInputConnector(vframe);						\
		m_pOutput = new COutputConnector(vframe);					\
		Resume();													\
	}																\
    void class_name::ShutDown()										\
	{																\
		CFilterGadget::ShutDown();									\
		delete m_pInput;											\
		m_pInput = NULL;											\
		delete m_pOutput;											\
		m_pOutput = NULL;											\
		delete_params;												\
	}																\

#define IMPLEMENT_VIDEO_FILTER_WITH_SETUP_EX(class_name, intype, outtype, lineage, plugin, init_params, delete_params)	\
	IMPLEMENT_RUNTIME_GADGET_EX(class_name, CFilterGadget, "Video." lineage, plugin);\
	class_name::class_name():										\
	m_pSetupDlg(NULL)												\
	{																\
		init_params;												\
		m_pInput = new CInputConnector(intype);						\
		m_pOutput = new COutputConnector(outtype);					\
		Resume();													\
	}																\
    void class_name::ShutDown()										\
	{				                                                \
        AFX_MANAGE_STATE(AfxGetStaticModuleState());                \
		if (m_pSetupDlg->GetSafeHwnd( ))                            \
            m_pSetupDlg->DestroyWindow();                           \
        if (m_pSetupDlg)                                            \
			m_pSetupDlg->Delete();									\
		m_pSetupDlg = NULL;											\
		CFilterGadget::ShutDown();									\
		delete m_pInput;											\
		m_pInput = NULL;											\
		delete m_pOutput;											\
		m_pOutput = NULL;											\
		delete_params;												\
	}																\
	BOOL class_name::IsSetupOn()									\
	{																\
		return ((m_pSetupDlg != NULL) && m_pSetupDlg->IsOn());		\
	}																\

#endif