namespace FLWPlayerSharp
{
    partial class FLWDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FLWDialog));
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.buttonRun = new System.Windows.Forms.Button();
            this.buttonSetup = new System.Windows.Forms.Button();
            this.buttonOpen = new System.Windows.Forms.Button();
            this.displayBox = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.displayBox)).BeginInit();
            this.SuspendLayout();
            // 
            // buttonOK
            // 
            this.buttonOK.Location = new System.Drawing.Point(456, 335);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(71, 36);
            this.buttonOK.TabIndex = 4;
            this.buttonOK.Text = "&OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // buttonStop
            // 
            this.buttonStop.Image = global::FLWPlayerSharp.Properties.Resources.Stop;
            this.buttonStop.Location = new System.Drawing.Point(95, 335);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(44, 36);
            this.buttonStop.TabIndex = 2;
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // buttonRun
            // 
            this.buttonRun.Image = global::FLWPlayerSharp.Properties.Resources.Run;
            this.buttonRun.Location = new System.Drawing.Point(48, 335);
            this.buttonRun.Name = "buttonRun";
            this.buttonRun.Size = new System.Drawing.Size(44, 36);
            this.buttonRun.TabIndex = 1;
            this.buttonRun.UseVisualStyleBackColor = true;
            this.buttonRun.Click += new System.EventHandler(this.buttonRun_Click);
            // 
            // buttonSetup
            // 
            this.buttonSetup.Image = global::FLWPlayerSharp.Properties.Resources.WRENCH;
            this.buttonSetup.Location = new System.Drawing.Point(162, 335);
            this.buttonSetup.Name = "buttonSetup";
            this.buttonSetup.Size = new System.Drawing.Size(44, 36);
            this.buttonSetup.TabIndex = 3;
            this.buttonSetup.UseVisualStyleBackColor = true;
            this.buttonSetup.Click += new System.EventHandler(this.buttonSetup_Click);
            // 
            // buttonOpen
            // 
            this.buttonOpen.Image = global::FLWPlayerSharp.Properties.Resources.FOLDER03;
            this.buttonOpen.ImageAlign = System.Drawing.ContentAlignment.BottomRight;
            this.buttonOpen.Location = new System.Drawing.Point(1, 335);
            this.buttonOpen.Name = "buttonOpen";
            this.buttonOpen.Size = new System.Drawing.Size(44, 36);
            this.buttonOpen.TabIndex = 0;
            this.buttonOpen.UseVisualStyleBackColor = true;
            this.buttonOpen.Click += new System.EventHandler(this.buttonOpen_Click);
            // 
            // displayBox
            // 
            this.displayBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.displayBox.Location = new System.Drawing.Point(1, -2);
            this.displayBox.Name = "displayBox";
            this.displayBox.Size = new System.Drawing.Size(526, 331);
            this.displayBox.TabIndex = 5;
            this.displayBox.TabStop = false;
            // 
            // FLWDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(527, 372);
            this.Controls.Add(this.displayBox);
            this.Controls.Add(this.buttonStop);
            this.Controls.Add(this.buttonRun);
            this.Controls.Add(this.buttonSetup);
            this.Controls.Add(this.buttonOpen);
            this.Controls.Add(this.buttonOK);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "FLWDialog";
            this.Text = "FLW Viewer";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.FLWDialog_FormClosed);
            this.Load += new System.EventHandler(this.FLWDialog_Load);
            ((System.ComponentModel.ISupportInitialize)(this.displayBox)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Button buttonOpen;
        private System.Windows.Forms.Button buttonSetup;
        private System.Windows.Forms.Button buttonRun;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.PictureBox displayBox;
    }
}

