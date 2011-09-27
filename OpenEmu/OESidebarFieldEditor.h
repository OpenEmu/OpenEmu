//
//  SidebarFieldEditor.h
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OESidebarFieldEditor : NSTextView {
@private
    NSRect clipRect;
}

+ (id)fieldEditor;

@end
