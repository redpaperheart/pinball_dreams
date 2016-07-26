/*
 * ObjectContainer.h
 *
 * Created by Daniel Scheibel on 10/25/12.
 *
 * Copyright (c) 2012, Red Paper Heart, All rights reserved.
 * This code is intended for use with the Cinder C++ library: http://libcinder.org
 *
 * To contact Red Paper Heart, email hello@redpaperheart.com or tweet @redpaperhearts
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once
#include "Object.h"

namespace rph {
    
    class ObjectContainer {
        
    public:
        ObjectContainer(){};
        ~ObjectContainer(){};
        
        virtual void update( float deltaTime = 0.0f, int beginIndex = 0, int endIndex = 0x7fffffff);
        virtual void draw(int beginIndex = 0, int endIndex = 0x7fffffff);
        
        void    addChild(Object *obj){ if(obj != NULL)mChildren.push_back(obj); }
        void    addChild( Object *obj, int index){ if(obj != NULL) mChildren.insert( mChildren.begin()+index, obj ); };
        
        int getNumChildren(){ return mChildren.size(); }
        
        Object*   getChildAt(int index){ return mChildren[index]; }
        std::vector<Object *>* getChildren(){ return &mChildren; }

        Object* removeChildAt(int index);
        void    removeChildren(int beginIndex = 0, int endIndex = 0x7fffffff);

        void            swapChildrenAt(int index1, int index2){};
        
        std::vector<Object *> mChildren;
    protected:
    private:
        
        
    };
    
}