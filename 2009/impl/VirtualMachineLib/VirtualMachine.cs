using System.Collections.Generic;
using System.IO;
using log4net;

namespace ICFP2009.VirtualMachineLib
{
    public class VirtualMachine
    {
        private static readonly ILog _log = LogManager.GetLogger(typeof (VirtualMachine));
        private static readonly object _virtualMachineMutex = new object();
        private static VirtualMachine _instance;

        private InstructionManager _instructionManager;
        private MemoryManager _memoryManager;

        /// <summary>
        /// ����� ����� �� ������.
        /// </summary>
        private VirtualMachine()
        {
        }

        public static VirtualMachine Instance
        {
            get
            {
                if (_instance == null)
                {
                    lock (_virtualMachineMutex)
                        if (_instance == null)
                            _instance = new VirtualMachine();
                }

                return _instance;
            }
        }

        public void LoadBinary(Stream binaryStream)
        {
            var instructions = new List<int>();
            var initialMemory = new List<double>();

            var binaryReader = new BinaryReader(binaryStream);

            _log.InfoFormat("Reading {0} bytes as image file...", binaryStream.Length);

            // ������ ��� ������ �� �����
            int frameIndex = 0;
            BinaryFrame frame = ReadFrame(binaryReader, frameIndex);
            while (frame != null)
            {
                instructions.Add(frame.Instruction);
                initialMemory.Add(frame.Memory);

                frame = ReadFrame(binaryReader, ++frameIndex);
            }

            _log.InfoFormat("Reader {0} frames.", frameIndex);

            _instructionManager = new InstructionManager(instructions);
            _memoryManager = new MemoryManager(initialMemory);
        }

        private static BinaryFrame ReadFrame(BinaryReader binaryReader, int frameIndex)
        {
            var frame = new BinaryFrame();

            if (binaryReader.BaseStream.Position >= binaryReader.BaseStream.Length)
                return null;

            // ���� ������ ������, �� ������� ���� �������� ������, � ����� ���������� ����
            // ����� ��������.
            if (frameIndex % 2 == 0)
            {
                frame.Memory = binaryReader.ReadDouble();
                frame.Instruction = binaryReader.ReadInt32();
            }
            else
            {
                frame.Instruction = binaryReader.ReadInt32();
                frame.Memory = binaryReader.ReadDouble();
            }

            return frame;
        }

        #region Nested type: BinaryFrame

        private class BinaryFrame
        {
            public int Instruction { get; set; }

            public double Memory { get; set; }
        }

        #endregion
    }
}