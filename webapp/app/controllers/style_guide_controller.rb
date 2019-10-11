class StyleGuideController < ApplicationController

  def page
    set_pages
    if params[:page_id].nil?
      @page = @pages.first
    else
      @page = @pages.find{|p| p[:slug] == params[:page_id] }
    end
    render @page[:slug]
  end

  def set_pages
    @pages = [
      { name: "Icons",                  slug: "icon"          },
      { name: "Logo",                   slug: "logo"          },
      { name: "Robot",                  slug: "robot"         },
      { name: "Typography",             slug: "typography"    },
      { name: "Badge",                  slug: "badge"         },
      { name: "Button",                 slug: "btn"           },
      { name: "Button Group",           slug: "btn_group"     },
      { name: "Dropdown",               slug: "dropdown"      },
      { name: "Form",                   slug: "form"          },
      { name: "Actions list",           slug: "actions_list"  },
      { name: "Table",                  slug: "table"         },
      { name: "Banner",                 slug: "banner"        },
      { name: "Card",                   slug: "card"          },
      { name: "Card list",              slug: "card_list"     },
      { name: "Note",                   slug: "note"          },
      { name: "Blankslate",             slug: "blankslate"    },
      { name: "Modal",                  slug: "modal"         },
      { name: "Tabs",                   slug: "tabs"          },
      { name: "Nav",                    slug: "nav"           },
      { name: "Agent header",           slug: "agent_header"  },
      { name: "Agent box",              slug: "agent_box"     },
      { name: "Agent compact",          slug: "agent_compact" },
      { name: "Interpretations list",   slug: "interpretations_list" },
      { name: "Entities list",          slug: "entities_list" },
      { name: "Chatbot",                slug: "chatbot"       },
      { name: "Console",                slug: "console"       },
      { name: "Markdown",               slug: "markdown"      }
    ]
  end

end
