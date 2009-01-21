using System;
using System.Collections;
using System.Collections.Generic;

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
        public void Add(int i)
        {
        }

        public int Length
        {
            get
            {
                return 0;
            }
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

        public IEnumerator<ItemType> GetEnumerator()
        {
            return new List<ItemType>().GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}