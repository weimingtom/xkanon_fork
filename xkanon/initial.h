/*  initial.h  : 
**
**  
**
**	static Init init0(pri, A);
**
**  
**  
**
**  
*/

/*
 *
 *  Copyright (C) 2002-   Kazunori Ueno(JAGARL) <jagarl@creator.club.ne.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
#ifndef __INITIAL_H__
#define __INITIAL_H__

#include<vector>

class InitInstanceBase;
class Initialize {
	typedef std::vector<InitInstanceBase*> ArrayType;
	ArrayType array;
	static Initialize* instance;
	static Initialize& Instance(void);
	Initialize(void) {}
public:
	static void Add(InitInstanceBase* datum);
	static void Exec(void);
};

struct InitInstanceBase {
	int priority_;
	InitInstanceBase(int pri) : priority_(pri) { }
	virtual void exec(void) = 0;
};
template<class _Function> struct InitInstance : InitInstanceBase {
	_Function func_;
	InitInstance(int priority, _Function __f) :
		InitInstanceBase(priority), func_(__f) {}
	void exec(void) { func_(); }
};

struct Init {
	template<class _Function> Init(int priority, _Function _f) {
		InitInstance<_Function>* instance = new InitInstance<_Function>(priority, _f);
		Initialize::Add(instance);
	}
};


template <class _Operation>
class binder0th {
protected:
  _Operation op;
  typename _Operation::argument_type value;
public:
  typedef typename _Operation::result_type result_type;

  binder0th(const _Operation& __x,
            const typename _Operation::argument_type& __y)
      : op(__x), value(__y) {}
  typename _Operation::result_type
  operator()(void) const {
    return op(value);
  }
};
template <class _Operation, class _Tp>
inline binder0th<_Operation>
bind0th(const _Operation& __oper, const _Tp& __x)
{
  typedef typename _Operation::argument_type _Arg1_type;
  return binder0th<_Operation>(__oper, _Arg1_type(__x));
}

#endif /* __INITIAL_H__ */
