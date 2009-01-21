using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;

namespace Common
{
    /// <summary>
    /// ������ ��������, �����������:
    ///  * ������� ������� � ����� ����� ������ ������ �������� ��� ��������� -- O(1).
    ///  * ������� ������ �� ������ �������� ������ -- O(1).
    ///  * ������� �������� ������ �������� ������ ��� ��������� -- �(1).
    /// 
    /// ������ ���������� ����������� ������� ������ �� ��������� �������. ������ �� ��������� �������� ������
    /// �������� ����������. ��������� ������ ������������ ��� ������� ��������� �������� (������, ��������, ��������).
    /// </summary>
    /// <typeparam name="ItemType">��� �������� ������.</typeparam>
    public class RoapList<ItemType> : IEnumerable<ItemType>
    {
        /// <summary>  ������ ������� �� �������� ��������. </summary>
        private const int _blockSize = 5;

        public int Length
        {
            get
            {
                return 0;
            }
        }

        public ItemType this[int index]
        {
            get
            {
                return default(ItemType);
            }

            set
            {
            }
        }

        #region IEnumerable<ItemType> Members

        public IEnumerator<ItemType> GetEnumerator()
        {
            return new List<ItemType>().GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        #endregion

        public void Add(int i)
        {
        }

        public void AddRange(ICollection<ItemType> list)
        {
        }

        public void RemoveAt(int i)
        {
        }

        public void RemoveRange(int i, int i1)
        {
        }

        #region Nested type: ListItem

        /// <summary>
        /// ������� ������, ��������������� �������� ��������.
        /// </summary>
        private class ListItem
        {
            public ListItem(int firstIndex)
            {
                Items = new ItemType[_blockSize];
                FirstIndex = firstIndex;
            }

            public ItemType[] Items
            {
                [DebuggerStepThrough]
                get;
                [DebuggerStepThrough]
                set;
            }

            public int FirstIndex
            {
                [DebuggerStepThrough]
                get;
                [DebuggerStepThrough]
                set;
            }
        }

        #endregion
    }
}