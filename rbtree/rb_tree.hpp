#ifndef _BB_TREE_H
#define _RB_TREE_H

#include <cstddef>
#include <sys/types.h>

#include <iostream>
#include <memory>
#include <cassert>

class RBNode;
class RBTree;
using SmartNodePtr = std::shared_ptr<RBNode>;

enum Color { BLACK = false, RED = true};
class RBNode {
private:
    int data_;
    SmartNodePtr left_;
    SmartNodePtr right_;
    SmartNodePtr parent_;
    enum Color color_;

public:
    RBNode(int data):
    data_(data), color_(RED) {}

    ~RBNode() { 
        std::cout << "Destructing:" << data_  
        << ", Color:" << (color_ == BLACK ? "Black" : "Red ") << std::endl; 
    }

    SmartNodePtr get_grandparent() 
    {
        if (!parent_)
            return SmartNodePtr(); 
        return parent_->parent_;
    }

    SmartNodePtr get_uncle() 
    {
        if (!get_grandparent()) 
            return SmartNodePtr(); 

        if (get_grandparent()->left_ == parent_)
            return get_grandparent()->right_;
        else
            return get_grandparent()->left_;
    }

    SmartNodePtr get_sibling() 
    {
        if (!parent_)
            return SmartNodePtr(); 
        
        if (parent_->left_.get() == this)
            return parent_->right_;
        else
            return parent_->left_;
    }

    friend class RBTree;
};


class RBTree {
public:
    SmartNodePtr root_;
    SmartNodePtr nill_; // shared by all
    RBTree()
    {
        nill_ = std::make_shared<RBNode>(-1);
        nill_->color_ = BLACK;
        nill_->left_ = nill_->right_ = nill_->parent_ = SmartNodePtr();
    }

    void insert(int data)
    {
        if (!root_)
        {
            root_ = std::make_shared<RBNode>(data);
            root_->color_ = BLACK;
            root_->parent_ = SmartNodePtr();
            root_->left_ = root_->right_ = nill_;
        }
        else
            insert(root_, data);
    }

    void insert(SmartNodePtr node, int data)
    {
        assert(root_);
        assert(data != node->data_);

        if (data < node->data_)
        {
            if (node->left_ != nill_)
                insert(node->left_, data);
            else
            {
                SmartNodePtr tmp = std::make_shared<RBNode>(data);
                tmp->left_ = tmp->right_ = nill_;
                tmp->parent_ = node;
                node->left_ = tmp;
                do_post_insert(tmp);
            }
        }
        else
        {
            if (node->right_ != nill_)
                insert(node->right_, data);
            else
            {
                SmartNodePtr tmp = std::make_shared<RBNode>(data);
                tmp->left_ = tmp->right_ = nill_;
                tmp->parent_ = node;
                node->right_ = tmp;
                do_post_insert(tmp);
            }
        }
    }

    void do_post_insert(SmartNodePtr node)
    {
        assert(node != nill_);
        
        if (! node->parent_) 
        {
            root_ = node;
            node->color_ = BLACK;
            return;
        }

        // 在黑节点下面插入红节点，总是平衡的
        if (node->parent_->color_ == BLACK)
            return; 
        
        // case 父节点和叔父节点都是RED
        // 将它们两个重绘为黑色并重绘祖父节点G为红色
        // 在祖父节点G上递归地进行情形1的整个过程
        if (node->get_uncle()->color_ == RED ) //总是返回有效指针，即使是nill_ 
        {
            node->parent_->color_ = BLACK;
            node->get_uncle()->color_ = BLACK;
            node->get_grandparent()->color_ = RED;
            return do_post_insert(node->get_grandparent());
        }

        //
        //叔父节点U是黑色或缺少
        assert(node->parent_->color_ == RED && node->get_uncle()->color_ == BLACK);
        assert(node->get_grandparent()->color_ == BLACK);

        if (     node == node->parent_->left_ && node->parent_ == node->get_grandparent()->left_) 
        { 
            node->parent_->color_ = BLACK;
            node->get_grandparent()->color_ = RED;

            rotate_right(node->parent_); 
        }
        else if (node == node->parent_->left_ && node->parent_ == node->get_grandparent()->right_) 
        {
            node->get_grandparent()->color_ = RED;
            node->color_ = BLACK;

            rotate_right(node);
            rotate_left(node);
        }
        else if (node == node->parent_->right_ && node->parent_ == node->get_grandparent()->right_) 
        {
            node->get_grandparent()->color_ = RED;
            node->parent_->color_ = BLACK;

            rotate_left(node->parent_);
        }        
        else if (node == node->parent_->right_ && node->parent_ == node->get_grandparent()->left_) 
        {
            node->get_grandparent()->color_ = RED;
            node->color_ = BLACK;

            rotate_left(node);
            rotate_right(node);
        }

    }
    
    // 约定旋转后都是返回旋转后的跟节点
    void rotate_left(SmartNodePtr node)
    {
        if (! node->parent_) 
        {
            root_ = node;
            node->color_ = BLACK;
            return;
        }

        SmartNodePtr gp = node->get_grandparent();
        SmartNodePtr fa = node->parent_;
        SmartNodePtr yp = node->left_;

        fa->right_ = yp;
        if(yp != nill_)
            yp->parent_ = fa;

        node->left_ = fa;
        fa->parent_ = node;

        if(root_ == fa)
            root_ = node;

        node->parent_ = gp;

        if(gp)
        {
            if(gp->left_ == fa)
                gp->left_ = node;
            else
                gp->right_ = node;
        }
    }

    void rotate_right(SmartNodePtr node)
    {
        if (! node->parent_) 
        {
            root_ = node;
            node->color_ = BLACK;
            return;
        }

        SmartNodePtr gp = node->get_grandparent();
        SmartNodePtr fa = node->parent_;
        SmartNodePtr yp = node->left_;

        fa->left_ = yp;
        if(yp != nill_)
            yp->parent_ = fa;

        node->right_ = fa;
        fa->parent_ = node;

        if(root_ == fa)
            root_ = node;

        node->parent_ = gp;

        if(gp)
        {
            if(gp->left_ == fa)
                gp->left_ = node;
            else
                gp->right_ = node;
        }
    }


    void display(SmartNodePtr node, int level)
    {
        if (!node)
            return;
        
        display(node->right_, level + 1);
        std::cout << std::endl;

        if (node == root_)
            std::cout << "Root -> ";

        for (size_t i = 0; i < level && node != root_; i++)
            std::cout << "        " ;

        std::cout << node->data_ << "[" << (node->color_ == BLACK ? "B" : "R ") << "]";
        display(node->left_, level + 1);
    }

    void pre_order(SmartNodePtr node)
    {
        if (!node) 
            return;

        std::cout<<node->data_<<", ";
        pre_order(node->left_);
        pre_order(node->right_);
    }

    void in_order(SmartNodePtr node)
    {
        if (!node) 
            return;

        in_order(node->left_);
        std::cout<<node->data_<<", ";
        in_order(node->right_);
    }

    void post_order(SmartNodePtr node)
    {
        if (!node) 
            return;

        post_order(node->left_);
        post_order(node->right_);
        std::cout<<node->data_<<", ";
    }
    
};

#endif //_RB_TREE_H
