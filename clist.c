/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   clist.c
 *
 * Author: Scott Heavner
 *
 *   Scoring is done using a GtkTreeView and handled in this file.
 *
 *   Variables are exported in gyahtzee.h
 *
 *   This file is largely based upon GTT code by Eckehard Berns.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <config.h>
#include <gdk/gdk.h>
#include <gnome.h>

#include <stdio.h>

#include "yahtzee.h"
#include "gyahtzee.h"

void
update_score_cell(GtkWidget *treeview, gint row, gint col, int val)
{
        GtkTreeModel *model;
        GtkTreeIter iter;
        char *buf;

        g_assert(treeview != NULL);

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
        gtk_tree_model_iter_nth_child(model, &iter, NULL, row);
        if (val < 0)
                buf = "";
        else
                buf = g_strdup_printf("%i", val);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, col, buf, -1);
        g_free(buf);
}

static void
set_label_bold(GtkLabel *label, gboolean make_bold)
{
        PangoAttrList *attrlist;
        PangoAttribute *attr;
        
        g_assert(label != NULL);

        attrlist = gtk_label_get_attributes(label);
        if (!attrlist)
                attrlist = pango_attr_list_new();

        if (make_bold) {
                attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
                attr->start_index = 0;
                attr->end_index = -1;
                pango_attr_list_change(attrlist, attr);
        } else {
                attr = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
                attr->start_index = 0;
                attr->end_index = -1;
                pango_attr_list_change(attrlist, attr);
        }
        gtk_label_set_attributes(label, attrlist);
}

/* Shows the active player by make the player name bold in the TreeView. */
void
ShowoffPlayer(GtkWidget *treeview, int player, int so)
{
        GtkTreeViewColumn *col;
        GtkWidget *label;
        GList *collist;
        
        g_return_if_fail(treeview != NULL);

        if (player < 0 || player >= MAX_NUMBER_OF_PLAYERS)
                return;

        collist = gtk_tree_view_get_columns(GTK_TREE_VIEW(treeview));
        col = GTK_TREE_VIEW_COLUMN(g_list_nth_data(collist, player+1));
        g_list_free(collist);

        label = gtk_tree_view_column_get_widget(col);
        if (!label)
                return;

        g_assert(GTK_IS_LABEL(label));

        set_label_bold(GTK_LABEL(label), so);
}

static void
row_activated_cb(GtkTreeView *treeview, GtkTreePath *path,
                 GtkTreeViewColumn *column, gpointer user_data)
{
        char *path_str;
        int row;

        path_str = gtk_tree_path_to_string(path);
        if (sscanf(path_str, "%i", &row) != 1) {
                g_warning("%s: could not convert '%s' to integer\n",
                          G_GNUC_FUNCTION, path_str);
                g_free(path_str);
                return;
        }
        g_free(path_str);
        switch (row) {
        case (R_UTOTAL):
        case (R_BONUS):
        case (R_BLANK1):
        case (R_GTOTAL):
        case (R_LTOTAL):
                break;
        default:
                /* Adjust for Upper Total / Bonus entries */
                if (row >= NUM_UPPER)
                        row -= 3;

                if (row < NUM_FIELDS && !players[CurrentPlayer].finished) {
                        if (play_score(CurrentPlayer, row) == SLOT_USED) {
                                say(_("Already used! "
                                      "Where do you want to put that?"));
                        } else {
                                NextPlayer();
                        }
                }
                break;
        }
}

static gboolean
activate_selected_row_idle_cb(gpointer data)
{
        GtkTreeView *tree = GTK_TREE_VIEW(data);
        GtkTreeViewColumn *column;
        GtkTreePath *path;

        path = NULL;
        gtk_tree_view_get_cursor(tree, &path, &column);
        if (path) {
                if (!column)
                        column = gtk_tree_view_get_column(tree, 0);
                gtk_tree_view_row_activated(tree, path, column);
        }
        
        return FALSE;
}

/* Returns: FALSE to let the GtkTreeView focus the selected row */
static gboolean
tree_button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
        GtkTreeView *tree = GTK_TREE_VIEW(data);

        g_assert(widget != NULL);
        g_assert(event != NULL);

        if (event->type != GDK_BUTTON_PRESS)
                return FALSE;

        g_idle_add_full(G_PRIORITY_HIGH, activate_selected_row_idle_cb,
                        (gpointer) tree, NULL);

        return FALSE;
}

GtkWidget *create_score_list(void)
{
        GtkWidget *tree;
        GtkListStore *store;
        
        store = gtk_list_store_new(MAX_NUMBER_OF_PLAYERS + 2,
                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                   G_TYPE_STRING, G_TYPE_STRING);
        tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree), TRUE);

        g_object_unref(store);

        g_signal_connect(G_OBJECT(tree), "row-activated",
                         G_CALLBACK(row_activated_cb), NULL);
        g_signal_connect(G_OBJECT(tree), "button-press-event",
                         G_CALLBACK(tree_button_press_cb), (gpointer) tree);

        return tree;
}

static void
add_columns(GtkTreeView *tree)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;
        GtkWidget *label;
        GValue *prop_value;
        int i;
        
        /* Create columns */
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("", renderer,
                                                          "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
        for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++) {
                renderer = gtk_cell_renderer_text_new();
                prop_value = g_new0(GValue, 1);
                g_value_init(prop_value, G_TYPE_FLOAT);
                g_value_set_float(prop_value, 1.0);
                g_object_set_property(G_OBJECT(renderer),
                                      "xalign", prop_value);
                g_value_unset(prop_value);
                g_free(prop_value);

                column = gtk_tree_view_column_new();
                gtk_tree_view_column_pack_start(column, renderer, TRUE);
                gtk_tree_view_column_set_attributes(column, renderer,
                                                    "text", i+1, NULL);
                gtk_tree_view_column_set_min_width(column, 95);
                gtk_tree_view_column_set_alignment(column, 1.0);
                label = gtk_label_new(players[i].name);
                gtk_tree_view_column_set_widget(column, label);
                gtk_widget_show(label);
                gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
        }
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("", renderer, "text",
                                                          MAX_NUMBER_OF_PLAYERS
                                                          + 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
}

void
score_list_set_column_title(GtkWidget *treeview, int column, const char *str)
{
        GtkTreeViewColumn *col;
        GtkWidget *label;

        g_assert(treeview != NULL);

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), column);
        label = gtk_tree_view_column_get_widget(col);
        if (!label)
                return;

        gtk_label_set_text(GTK_LABEL(label), str);
}

static void
initialize_column_titles(GtkTreeView *treeview)
{
        GtkTreeViewColumn *col;
        GList *collist, *node;
        GtkWidget *label;
        int i;

        collist = gtk_tree_view_get_columns(treeview);
        i = 0;
        for (node = collist; node != NULL; node = g_list_next(node)) {
                col = GTK_TREE_VIEW_COLUMN(node->data);
                label = gtk_tree_view_column_get_widget(col);
                if (!label)
                        continue;

                if (i < NumberOfPlayers)
                        gtk_label_set_text(GTK_LABEL(label), players[i].name);
                else
                        gtk_label_set_text(GTK_LABEL(label), "");
                i++;
        }
        g_list_free(collist);
}

void setup_score_list(GtkWidget *treeview)
{
        GtkTreeModel *model;
        GtkListStore *store;
        GtkTreeIter iter;
        GList *columns;
        int i;

        g_assert(treeview != NULL);

        columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(treeview));
        if (!columns) {
                add_columns(GTK_TREE_VIEW(treeview));
        } else {
                initialize_column_titles(GTK_TREE_VIEW(treeview));
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);

        for (i = 0; i < NUM_UPPER; i++) {
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, _(FieldLabels[i]), -1);
        }

        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _(FieldLabels[F_BONUS]), -1);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _(FieldLabels[F_UPPERT]), -1);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, "", -1);

        for (i = 0; i < NUM_LOWER; i++) {
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter,
                                   0, _(FieldLabels[i+NUM_UPPER]),
                                   -1);
        }

        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _(FieldLabels[F_LOWERT]), -1);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _(FieldLabels[F_GRANDT]), -1);
}	

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/   
