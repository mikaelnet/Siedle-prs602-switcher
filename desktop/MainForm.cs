using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SiedleInputSwitcher
{
    public partial class MainForm : Form
    {
        private readonly Color _defaultColor;
        private delegate void DataReceivedDelegate(string str);
        private readonly DataReceivedDelegate _dataReceived;

        public MainForm()
        {
            InitializeComponent();
            LoadComPorts();
            MoveDisconnectButton();
            UpdateControls();
            _defaultColor = buttonA.BackColor;
            _dataReceived = ReceivedData;
        }

        public void LoadComPorts()
        {
            comPortDropdown.Items.Clear();

            var configuredPort = Properties.Settings.Default["ComPort"];
            foreach (var comPortName in SerialPort.GetPortNames())
            {
                comPortDropdown.Items.Add(comPortName);
                if (string.Equals(comPortName, configuredPort))
                {
                    comPortDropdown.SelectedItem = comPortName;
                }
            }
        }

        private void MoveDisconnectButton()
        {
            disconnectButton.Left = connectButton.Left;
            disconnectButton.Top = connectButton.Top;
            disconnectButton.Width = connectButton.Width;
            disconnectButton.Height = connectButton.Height;
            disconnectButton.Visible = false;
        }

        private void UpdateControls()
        {
            var connected = serialPort1.IsOpen;

            buttonA.Enabled = connected;
            buttonB.Enabled = connected;
            buttonC.Enabled = connected;
            offButton.Enabled = connected;

            comPortDropdown.Enabled = !connected;
        }

        private void exitButton_Click(object sender, EventArgs e)
        {
            disconnectButton_Click(sender, e);
            Application.Exit();
        }

        private void connectButton_Click(object sender, EventArgs e)
        {
            var port = comPortDropdown.SelectedItem as string;
            if (string.IsNullOrWhiteSpace(port))
                return;

            serialPort1.PortName = port;
            serialPort1.Open();

            disconnectButton.Visible = true;
            connectButton.Visible = false;

            SetColor(-1);

            UpdateControls();
        }

        private void disconnectButton_Click(object sender, EventArgs e)
        {
            disconnectButton.Visible = false;
            connectButton.Visible = true;

            if (!serialPort1.IsOpen)
                return;
            
            SendCommand("0");

            serialPort1.Close();
            Properties.Settings.Default["ComPort"] = serialPort1.PortName;
            Properties.Settings.Default.Save();

            UpdateControls();
        }

        private void SendCommand(string command)
        {
            if (!serialPort1.IsOpen)
                return;

            serialPort1.WriteLine(command);
        }

        private void SetRelay(int relay)
        {
            if (relay < 0 || relay > 3)
                return;

            char cmd = (char) ('0' + relay);
            SendCommand($"{cmd}");
        }

        private void SetColor(int relay)
        {
            var buttons = new[] {offButton, buttonA, buttonB, buttonC};
            foreach (var btn in buttons)
            {
                btn.BackColor = _defaultColor;
            }

            if (relay < 0 || relay > 3)
                return;

            buttons[relay].BackColor = relay == 0 ? Color.LightCoral : Color.LightGreen;
        }

        private void buttonA_Click(object sender, EventArgs e)
        {
            SetRelay(1);
        }

        private void buttonB_Click(object sender, EventArgs e)
        {
            SetRelay(2);
        }

        private void buttonC_Click(object sender, EventArgs e)
        {
            SetRelay(3);
        }

        private void offButton_Click(object sender, EventArgs e)
        {
            SetRelay(0);
        }

        private void ReceivedData(string str)
        {
            consoleTextBox.AppendText(str);
            bool isComment = false;
            foreach (char ch in str)
            {
                if (ch == '\r' || ch == '\n')
                {
                    isComment = false;
                    continue;
                }

                if (ch == '#')
                {
                    isComment = true;
                    continue;
                }

                // Ignore all characters in a comment block
                if (isComment)
                    continue;

                if (ch >= '0' && ch <= '3')
                {
                    int relay = ch - '0';
                    SetColor(relay);
                }
            }
        }

        private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            var serialPort = (SerialPort) sender;
            string data = serialPort.ReadExisting();
            Invoke(_dataReceived, data);
        }
    }
}
