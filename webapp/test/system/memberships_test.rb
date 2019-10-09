require "application_system_test_case"

class MembershipsTest < ApplicationSystemTestCase

  test "Display sharing overview" do
    admin_go_to_agents_index

    first(".dropdown__trigger > button").click
    click_link "Share"

    within(".modal") do
      expected = [
        "admin@viky.ai",
        "show_on_agent_weather@viky.ai",
        "edit_on_agent_weather@viky.ai"
      ]
      assert_equal expected, all(".actions-list li .user__info small").collect(&:text)

      expected = [
        "Collaborator Show",
        "Collaborator Edit"
      ]
      assert_equal expected, all(".actions-list li .user__info .merged-badges").collect(&:text)

      expected = [
        "Can show",
        "Can edit"
      ]
      assert_equal expected, all(".actions-list li .dropdown .btn").collect(&:text)
    end
  end


  test "update collaborator right" do
    admin_go_to_agents_index

    first(".dropdown__trigger > button").click
    click_link "Share"

    within(".modal") do
      first(".dropdown__trigger > button").click
      within(".dropdown__content") do
        click_link "Can edit"
      end
    end

    assert has_text?("show_on_agent_weather can now edit My awesome weather bot agent.")
    within(".modal") do
      expected = [
        "Collaborator Edit",
        "Collaborator Edit"
      ]
      assert_equal expected, all(".actions-list li .user__info .merged-badges").collect(&:text)
    end
  end


  test "Try to delete collaborator right and cancel" do
    admin_go_to_agents_index

    first(".dropdown__trigger > button").click
    click_link "Share"

    within(".modal") do
      first(".dropdown__trigger > button").click
      within(".dropdown__content") do
        click_link "Delete"
      end
      assert has_text?(
        "You're about to delete show_on_agent_weather rights on agent My awesome weather bot."
      )
      click_link "Cancel"
      assert has_text?("My awesome weather bot agent sharing options")
    end
  end


  test "Delete collaborator" do
    admin_go_to_agents_index

    first(".dropdown__trigger > button").click
    click_link "Share"

    within(".modal") do
      first(".dropdown__trigger > button").click
      within(".dropdown__content") do
        click_link "Delete"
      end
      assert has_text?(
        "You're about to delete show_on_agent_weather rights on agent My awesome weather bot."
      )
      click_button "Delete"
    end
    assert has_text?("show_on_agent_weather has successfully been removed from colaborators.")

    within(".modal") do
      expected = [
        "admin@viky.ai",
        "edit_on_agent_weather@viky.ai"
      ]
      assert_equal expected, all(".actions-list li .user__info small").collect(&:text)
    end
  end


  test "Share agent with multiple users at the same time" do
    admin_go_to_agents_index

    agent_terminator_dropdown = all(".dropdown__trigger > button").last
    agent_terminator_dropdown.click
    click_link "Share"
    within(".modal") do
      click_link "Invite collaborators"
      assert has_text?("Share with")
      fill_in "users-input", with: "#{users('confirmed').username}, #{users('show_on_agent_weather').email}"
      click_button "Invite"
    end
    assert has_text?("Agent terminator shared with : confirmed, show_on_agent_weather.")
  end


  test "Share agent with multiple users at the same time with one error" do
    admin_go_to_agents_index

    first(".dropdown__trigger > button").click
    click_link "Share"
    within(".modal") do
      click_link "Invite collaborators"
      assert has_text?("Share with")
      fill_in "users-input", with: "#{users('confirmed').email}, #{users('show_on_agent_weather').email}"
      click_button "Invite"
      assert has_text?("Agent is already shared with user: show_on_agent_weather@viky.ai.")
    end
  end
end
